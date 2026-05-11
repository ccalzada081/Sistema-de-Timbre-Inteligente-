#include <iostream>
#include <filesystem>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <opencv2/opencv.hpp>

using namespace cv;
namespace fs = std::filesystem;

#define HEIGHT 480
#define WIDTH  640
#define FPS    30

#define INPUT_DEVICE "/dev/input/event1"
#define LOCAL_CAPTURE_DIR "/root/proyecto/capturas"

#define WINDOWS_USER "carlo"
#define WINDOWS_IP "169.254.171.57"
#define WINDOWS_INCOMING_PATH "/C:/Users/carlo/registro_facial/incoming"

#define KEY_PLAY_CODE      207
#define KEY_NEXT_CODE      407
#define KEY_PREVIOUS_CODE  412

std::string generarNombreArchivo() {
    std::time_t ahora = std::time(nullptr);
    std::tm* tiempoLocal = std::localtime(&ahora);

    char buffer[128];
    std::strftime(buffer, sizeof(buffer), "foto_%Y%m%d_%H%M%S.jpg", tiempoLocal);

    return std::string(buffer);
}

std::string generarRutaLocal(const std::string& nombreArchivo) {
    return std::string(LOCAL_CAPTURE_DIR) + "/" + nombreArchivo;
}

void dibujarOverlay(Mat& img, const std::string& linea1, const std::string& linea2, const Scalar& colorTexto) {
    rectangle(img, Point(20, 20), Point(img.cols - 20, 130), Scalar(0, 0, 0), FILLED);
    rectangle(img, Point(20, 20), Point(img.cols - 20, 130), Scalar(255, 255, 255), 2);

    putText(img, linea1, Point(40, 70), FONT_HERSHEY_SIMPLEX, 1.2, colorTexto, 3);
    putText(img, linea2, Point(40, 115), FONT_HERSHEY_SIMPLEX, 0.9, Scalar(255, 255, 255), 2);
}

bool abrirCamara(VideoCapture& cam) {
    cam.open(0);

    if (!cam.isOpened()) {
        std::cerr << "Error: no se pudo abrir la camara." << std::endl;
        return false;
    }

    cam.set(CAP_PROP_FRAME_WIDTH, WIDTH);
    cam.set(CAP_PROP_FRAME_HEIGHT, HEIGHT);
    cam.set(CAP_PROP_FPS, FPS);

    return true;
}

void prepararVentana() {
    namedWindow("Vista previa de camara", WINDOW_NORMAL);
    resizeWindow("Vista previa de camara", WIDTH, HEIGHT);
    setWindowProperty("Vista previa de camara", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
}

bool cuentaRegresivaYCaptura(Mat& frameFinal, const Scalar& colorTexto) {
    VideoCapture cam;
    if (!abrirCamara(cam)) {
        return false;
    }

    prepararVentana();

    Mat preview;

    std::cout << "Camara encendida. Preparando captura..." << std::endl;

    for (int i = 10; i >= 1; --i) {
        cam >> preview;
        if (preview.empty()) {
            continue;
        }

        resize(preview, preview, Size(WIDTH, HEIGHT));
        dibujarOverlay(preview, "Mire a la camara", "Captura en " + std::to_string(i) + " segundos", colorTexto);

        imshow("Vista previa de camara", preview);
        waitKey(1000);
    }

    cam >> frameFinal;

    if (frameFinal.empty()) {
        std::cerr << "Error: no se pudo capturar frame." << std::endl;
        cam.release();
        destroyAllWindows();
        return false;
    }

    resize(frameFinal, frameFinal, Size(WIDTH, HEIGHT));

    Mat confirmacion = frameFinal.clone();
    dibujarOverlay(confirmacion, "Imagen capturada", "Enviando por SCP...", Scalar(0, 255, 255));
    imshow("Vista previa de camara", confirmacion);
    waitKey(1200);

    cam.release();
    destroyAllWindows();
    return true;
}

bool guardarImagen(const Mat& frame, const std::string& rutaSalida) {
    fs::create_directories(LOCAL_CAPTURE_DIR);

    if (!imwrite(rutaSalida, frame)) {
        std::cerr << "Error: no se pudo guardar la imagen en " << rutaSalida << std::endl;
        return false;
    }

    std::cout << "Imagen guardada localmente en: " << rutaSalida << std::endl;
    return true;
}

bool enviarImagenPorScp(const std::string& rutaLocal) {
    char comando[1024];

    std::snprintf(
        comando,
        sizeof(comando),
        "scp \"%s\" %s@%s:%s",
        rutaLocal.c_str(),
        WINDOWS_USER,
        WINDOWS_IP,
        WINDOWS_INCOMING_PATH
    );

    std::cout << "Ejecutando SCP..." << std::endl;
    std::cout << "Te pedira la contrasena de Windows." << std::endl;

    int resultado = std::system(comando);

    if (resultado == 0) {
        std::cout << "Imagen enviada correctamente a Windows." << std::endl;
        return true;
    }

    std::cerr << "Error: fallo al enviar imagen por scp." << std::endl;
    return false;
}

bool capturarYEnviar() {
    Mat frame;
    if (!cuentaRegresivaYCaptura(frame, Scalar(0, 255, 0))) {
        return false;
    }

    std::string nombreArchivo = generarNombreArchivo();
    std::string rutaLocal = generarRutaLocal(nombreArchivo);

    if (!guardarImagen(frame, rutaLocal)) {
        return false;
    }

    if (!enviarImagenPorScp(rutaLocal)) {
        return false;
    }

    return true;
}

int main() {
    fs::create_directories(LOCAL_CAPTURE_DIR);

    int fd = open(INPUT_DEVICE, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "Error: no se pudo abrir " << INPUT_DEVICE << std::endl;
        return -1;
    }

    std::cout << "Sistema listo." << std::endl;
    std::cout << "PLAY     -> capturar y enviar" << std::endl;
    std::cout << "PREVIOUS -> capturar y enviar" << std::endl;
    std::cout << "NEXT     -> salir" << std::endl;

    bool running = true;

    while (running) {
        struct input_event ev;
        ssize_t n = read(fd, &ev, sizeof(ev));

        while (n == sizeof(ev)) {
            if (ev.type == EV_KEY && ev.value == 1) {
                switch (ev.code) {
                    case KEY_PLAY_CODE:
                        std::cout << "[BOTON PLAY] Capturando y enviando..." << std::endl;
                        capturarYEnviar();
                        std::cout << "Sistema listo." << std::endl;
                        break;

                    case KEY_PREVIOUS_CODE:
                        std::cout << "[BOTON PREVIOUS] Capturando y enviando..." << std::endl;
                        capturarYEnviar();
                        std::cout << "Sistema listo." << std::endl;
                        break;

                    case KEY_NEXT_CODE:
                        std::cout << "[BOTON NEXT] Saliendo..." << std::endl;
                        running = false;
                        break;

                    default:
                        break;
                }
            }

            n = read(fd, &ev, sizeof(ev));
        }

        usleep(10000);
    }

    close(fd);
    return 0;
}
