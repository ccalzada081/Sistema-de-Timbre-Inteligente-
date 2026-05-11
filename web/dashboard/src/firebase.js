import { initializeApp } from "firebase/app";
import { getFirestore } from "firebase/firestore";

const firebaseConfig = {
  apiKey: "AIzaSyBLfitE0vYSiNyFnV89MTgbJPpVs9JxdSQ",
  authDomain: "registro-facial-3d07f.firebaseapp.com",
  projectId: "registro-facial-3d07f",
  storageBucket: "registro-facial-3d07f.firebasestorage.app",
  messagingSenderId: "800322537407",
  appId: "1:800322537407:web:57c108de5ae04aa67a030a"
};

const app = initializeApp(firebaseConfig);

export const db = getFirestore(app);
