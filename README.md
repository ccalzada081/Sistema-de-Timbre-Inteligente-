# Sistema-de-Timbre-Inteligente-
Reconocimiento Facial y Registro 
---

# Descripción

Este proyecto implementa un sistema de captura y reconocimiento facial conectado a una tarjeta embebida Linux.

Cuando el usuario presiona un botón físico:

1. La cámara captura una imagen.
2. La imagen se transfiere mediante SCP hacia el host Windows.
3. Python procesa la imagen usando OpenCV.
4. El sistema detecta e identifica rostros registrados.
5. Los resultados se almacenan en Firebase.
6. El dashboard web muestra los registros en tiempo real.

---

# Arquitectura del Sistema

```text
Botón físico (i.MX8)
        │
        ▼
Captura de imagen (OpenCV C++)
        │
        ▼
Transferencia SCP
        │
        ▼
Backend Python + OpenCV
        │
 ┌──────┴──────┐
 ▼             ▼
Firebase     Dashboard React
(Storage + DB)
```

---

# Tecnologías Utilizadas

## Backend
- Python 3
- OpenCV
- Firebase Admin SDK
- NumPy

## Frontend
- React
- Vite
- CSS

## Embedded / Linux
- C++
- OpenCV C++
- SCP
- Makefile
- Linux Embedded (i.MX8)

## Cloud
- Firebase Storage
- Firebase Firestore

---

# Funcionalidades

- Captura de imágenes mediante botones físicos
- Transferencia automática de imágenes por SCP
- Detección facial en tiempo real
- Reconocimiento facial utilizando LBPH
- Registro de personas conocidas
- Dashboard web moderno
- Almacenamiento en Firebase
- Historial de registros
- Procesamiento automático de imágenes
- Integración Embedded + Cloud + Web

---

# Estructura del Proyecto

```text
Sistema-de-Timbre-Inteligente-
│
├── app.py
├── requirements.txt
├── registro.json
├── .gitignore
│
├── known_faces/
├── incoming/
├── processed/
├── results/
│
├── VM/
│   ├── camara.cpp
│   └── Makefile
│
└── web/
    └── dashboard/
        ├── src/
        ├── public/
        ├── package.json
        └── vite.config.js
```

---

# Flujo del Sistema

## 1. Captura

La tarjeta i.MX8 ejecuta:

```bash
./camara
```

El botón físico activa la captura de imagen.

---

## 2. Transferencia

La imagen se envía automáticamente al host Windows mediante SCP.

---

## 3. Procesamiento

El backend ejecuta:

```bash
python app.py
```

El sistema:
- detecta rostros
- identifica personas registradas
- genera resultados
- almacena datos en Firebase

---

## 4. Visualización

El dashboard React muestra:
- imagen procesada
- nombre identificado
- estado
- confianza
- historial

---

# Instalación

## Backend

Crear entorno virtual:

```bash
python -m venv venv
```

Activar entorno:

```bash
venv\Scripts\activate
```

Instalar dependencias:

```bash
pip install -r requirements.txt
```

---

## Frontend

```bash
cd web/dashboard
npm install
npm run dev
```

Abrir:

```text
http://localhost:5173
```

---

# Compilación Embedded

Dentro de la tarjeta Linux:

```bash
make
```

Ejecutar:

```bash
./camara
```

---

# Reconocimiento Facial

El sistema utiliza:
- Haar Cascades para detección facial
- LBPH Face Recognizer para identificación

Las imágenes de entrenamiento se almacenan en:

```text
known_faces/
```

Formato recomendado:

```text
Carlos Calzada1.jpg
Carlos Calzada2.jpg
Luis Arturo1.jpg
Luis Arturo2.jpg
```

---

# Firebase

El proyecto utiliza:
- Firebase Storage para imágenes
- Firestore para registros

El archivo:

```text
firebase_key.json
```

NO se incluye en el repositorio por seguridad.

---

# Resultados

El sistema permite:

- identificación automática
- monitoreo en tiempo real
- integración embedded-cloud
- almacenamiento histórico
- dashboard visual moderno
