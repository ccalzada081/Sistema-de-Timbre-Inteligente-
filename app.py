import os
import time
import json
import shutil
from datetime import datetime

import cv2
import numpy as np

import firebase_admin
from firebase_admin import credentials, firestore, storage

# ================== RUTAS ==================
BASE_DIR = r"C:\Users\carlo\registro_facial"

INPUT_DIR = os.path.join(BASE_DIR, "incoming")
PROCESSED_DIR = os.path.join(BASE_DIR, "processed")
RESULTS_DIR = os.path.join(BASE_DIR, "results")
KNOWN_FACES_DIR = os.path.join(BASE_DIR, "known_faces")

LOG_FILE = os.path.join(BASE_DIR, "registro.json")

PUBLIC_DIR = os.path.join(BASE_DIR, "web", "dashboard", "public")
PUBLIC_RESULTS_DIR = os.path.join(PUBLIC_DIR, "results")
PUBLIC_LOG_FILE = os.path.join(PUBLIC_DIR, "registro.json")

FIREBASE_KEY = os.path.join(BASE_DIR, "firebase_key.json")
STORAGE_BUCKET = "registro-facial-3d07f.firebasestorage.app"

VALID_EXTENSIONS = (".jpg", ".jpeg", ".png")

# ================== CREAR CARPETAS ==================
os.makedirs(INPUT_DIR, exist_ok=True)
os.makedirs(PROCESSED_DIR, exist_ok=True)
os.makedirs(RESULTS_DIR, exist_ok=True)
os.makedirs(KNOWN_FACES_DIR, exist_ok=True)
os.makedirs(PUBLIC_RESULTS_DIR, exist_ok=True)

# ================== FIREBASE ==================
firebase_ok = False
try:
    if not firebase_admin._apps:
        cred = credentials.Certificate(FIREBASE_KEY)
        firebase_admin.initialize_app(cred, {"storageBucket": STORAGE_BUCKET})

    db = firestore.client()
    bucket = storage.bucket()
    firebase_ok = True
    print("Firebase OK")
except Exception as e:
    print("Firebase ERROR:", e)

# ================== DETECTOR ==================
face_cascade = cv2.CascadeClassifier(
    cv2.data.haarcascades + "haarcascade_frontalface_default.xml"
)

# ================== ENTRENAR LBPH ==================
recognizer = cv2.face.LBPHFaceRecognizer_create()

label_map = {}
faces_data = []
labels = []

def entrenar_modelo():
    global label_map, faces_data, labels

    label_map = {}
    faces_data = []
    labels = []

    label_id = 0

    for archivo in os.listdir(KNOWN_FACES_DIR):
        if not archivo.lower().endswith(VALID_EXTENSIONS):
            continue

        ruta = os.path.join(KNOWN_FACES_DIR, archivo)
        nombre = os.path.splitext(archivo)[0]

        img = cv2.imread(ruta)
        if img is None:
            continue

        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        faces = face_cascade.detectMultiScale(gray, 1.3, 5)

        for (x, y, w, h) in faces:
            rostro = gray[y:y+h, x:x+w]
            rostro = cv2.resize(rostro, (160, 160))

            faces_data.append(rostro)
            labels.append(label_id)

        label_map[label_id] = nombre
        label_id += 1

    if len(faces_data) > 0:
        recognizer.train(faces_data, np.array(labels))
        print("Modelo entrenado con", len(label_map), "personas")
    else:
        print("No hay rostros para entrenar")

entrenar_modelo()

# ================== FIREBASE ==================
def subir_imagen(local_path, name):
    if not firebase_ok:
        return None
    try:
        blob = bucket.blob(f"resultados/{name}")
        blob.upload_from_filename(local_path)
        blob.make_public()
        return blob.public_url
    except Exception as e:
        print("Error Firebase:", e)
        return None

def guardar_firestore(data):
    if not firebase_ok:
        return False
    try:
        db.collection("registros").add(data)
        return True
    except:
        return False

# ================== JSON ==================
def guardar_json(data, path):
    registros = []

    if os.path.exists(path):
        try:
            with open(path, "r", encoding="utf-8") as f:
                contenido = f.read().strip()
                if contenido:
                    registros = json.loads(contenido)
        except:
            registros = []

    registros.append(data)

    with open(path, "w", encoding="utf-8") as f:
        json.dump(registros, f, indent=4, ensure_ascii=False)

# ================== PROCESAR ==================
def procesar_imagen(path):
    filename = os.path.basename(path)
    timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")

    # 🔥 SOLUCIÓN ERROR SCP (esperar archivo)
    time.sleep(1)

    img = None
    for _ in range(5):
        img = cv2.imread(path)
        if img is not None:
            break
        time.sleep(1)

    if img is None:
        print(f"No se pudo leer la imagen: {filename}")
        return

    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    faces = face_cascade.detectMultiScale(gray, 1.3, 5)

    persona = "No se pudo registrar"
    detalle = "Sin rostro"

    if len(faces) == 0:
        estado = "Sin rostro detectado"
    else:
        estado = "Rostro detectado"

        (x, y, w, h) = max(faces, key=lambda f: f[2]*f[3])

        rostro = gray[y:y+h, x:x+w]
        rostro = cv2.resize(rostro, (160, 160))

        label, confidence = recognizer.predict(rostro)

        if confidence < 60:
            persona = label_map[label]
            detalle = f"Confianza: {confidence:.2f}"
        else:
            persona = "Persona no registrada"
            detalle = f"Confianza baja: {confidence:.2f}"

        cv2.rectangle(img, (x,y), (x+w,y+h), (0,255,0), 2)
        cv2.putText(img, persona, (x, y-10), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255,255,255), 2)

    out_name = f"resultado_{timestamp}_{filename}"

    out_path = os.path.join(RESULTS_DIR, out_name)
    public_path = os.path.join(PUBLIC_RESULTS_DIR, out_name)

    cv2.imwrite(out_path, img)
    shutil.copy2(out_path, public_path)

    url = subir_imagen(out_path, out_name)

    log = {
        "imagen": filename,
        "fecha": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "rostros_detectados": int(len(faces)),
        "estado": estado,
        "persona": persona,
        "detalle_identificacion": detalle,
        "resultado": f"results/{out_name}",
        "url_imagen": url
    }

    guardar_firestore(log)
    guardar_json(log, LOG_FILE)
    guardar_json(log, PUBLIC_LOG_FILE)

    shutil.move(path, os.path.join(PROCESSED_DIR, filename))

    print("\n--- RESULTADO ---")
    print("Persona:", persona)
    print("Detalle:", detalle)

# ================== LOOP ==================
def loop():
    print("Sistema activo...")

    while True:
        archivos = os.listdir(INPUT_DIR)

        for f in archivos:
            ruta = os.path.join(INPUT_DIR, f)

            if not f.lower().endswith(VALID_EXTENSIONS):
                continue

            procesar_imagen(ruta)

        time.sleep(2)

if __name__ == "__main__":
    loop()
