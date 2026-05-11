import { useEffect, useState } from "react";
import "./App.css";

function App() {
  const [registros, setRegistros] = useState([]);
  const [error, setError] = useState("");

  const cargarRegistros = () => {
    fetch("/registro.json", { cache: "no-store" })
      .then((res) => {
        if (!res.ok) throw new Error("No se pudo cargar registro.json");
        return res.json();
      })
      .then((data) => {
        const lista = Array.isArray(data) ? data : [];
        setRegistros([...lista].reverse());
      })
      .catch((err) => {
        console.error(err);
        setError("No se pudo cargar registro.json");
      });
  };

  useEffect(() => {
    cargarRegistros();

    const intervalo = setInterval(() => {
      cargarRegistros();
    }, 3000);

    return () => clearInterval(intervalo);
  }, []);

  const eliminarRegistro = (indexEliminar) => {
    const confirmar = window.confirm("¿Deseas eliminar este registro del dashboard?");
    if (!confirmar) return;

    setRegistros((prev) => prev.filter((_, index) => index !== indexEliminar));
  };

  const obtenerImagen = (registro) => {
    if (!registro.resultado) return null;

    const partes = String(registro.resultado).split(/[\\/]/);
    const nombreArchivo = partes[partes.length - 1].trim();

    return `/results/${encodeURIComponent(nombreArchivo)}`;
  };

  const total = registros.length;

  const identificados = registros.filter(
    (r) =>
      r.persona &&
      r.persona !== "Persona no registrada" &&
      r.persona !== "No se pudo registrar"
  ).length;

  const noIdentificados = registros.filter(
    (r) =>
      !r.persona ||
      r.persona === "Persona no registrada" ||
      r.persona === "No se pudo registrar"
  ).length;

  return (
    <main className="page">
      <section className="hero">
        <p className="eyebrow">Sistema de timbre inteligente</p>
        <h1>Registro Facial</h1>
        <p className="subtitle">
          Monitoreo de capturas procesadas mediante OpenCV.
        </p>
      </section>

      <section className="stats">
        <div className="stat-card">
          <span>Total de registros</span>
          <strong>{total}</strong>
        </div>

        <div className="stat-card">
          <span>Personas identificadas</span>
          <strong>{identificados}</strong>
        </div>

        <div className="stat-card">
          <span>Sin identificación</span>
          <strong>{noIdentificados}</strong>
        </div>
      </section>

      <section className="records">
        <div className="section-header">
          <h2>Capturas procesadas</h2>
          <p>Últimos registros generados por el sistema.</p>
        </div>

        {error && <div className="empty">{error}</div>}

        {!error && registros.length === 0 && (
          <div className="empty">
            <h3>No hay registros disponibles</h3>
            <p>Ejecuta una captura desde la tarjeta.</p>
          </div>
        )}

        <div className="grid">
          {registros.map((registro, index) => {
            const imagen = obtenerImagen(registro);
            const rostros = Number(registro.rostros_detectados) || 0;
            const persona = registro.persona || "No se pudo registrar";

            const personaIdentificada =
              persona !== "Persona no registrada" &&
              persona !== "No se pudo registrar" &&
              rostros > 0;

            return (
              <article className="card" key={`${registro.imagen}-${index}`}>
                <div className="image-wrap">
                  {imagen ? (
                    <img
                      src={imagen}
                      alt={registro.imagen || "Resultado procesado"}
                      onError={(e) => {
                        e.currentTarget.style.display = "none";
                        e.currentTarget.parentElement.innerHTML =
                          '<div class="no-image">Imagen no disponible</div>';
                      }}
                    />
                  ) : (
                    <div className="no-image">Sin imagen</div>
                  )}
                </div>

                <div className="card-body">
                  <div className="status-row">
                    <span className={personaIdentificada ? "badge success" : "badge danger"}>
                      {personaIdentificada ? "Persona identificada" : persona}
                    </span>
                    <span className="faces">{rostros} rostro(s)</span>
                  </div>

                  <h3>{persona}</h3>

                  <dl>
                    <div>
                      <dt>Imagen original</dt>
                      <dd>{registro.imagen || "N/A"}</dd>
                    </div>

                    <div>
                      <dt>Estado</dt>
                      <dd>{registro.estado || "Sin estado"}</dd>
                    </div>

                    <div>
                      <dt>Detalle</dt>
                      <dd>{registro.detalle_identificacion || "Sin detalle"}</dd>
                    </div>

                    <div>
                      <dt>Archivo resultado</dt>
                      <dd>
                        {registro.resultado
                          ? String(registro.resultado).split(/[\\/]/).pop()
                          : "N/A"}
                      </dd>
                    </div>

                    {registro.url_imagen && (
                      <div>
                        <dt>Firebase Storage</dt>
                        <dd>
                          <a href={registro.url_imagen} target="_blank" rel="noreferrer">
                            Ver imagen
                          </a>
                        </dd>
                      </div>
                    )}
                  </dl>

                  <button
                    className="delete-button"
                    onClick={() => eliminarRegistro(index)}
                  >
                    Eliminar registro
                  </button>
                </div>
              </article>
            );
          })}
        </div>
      </section>
    </main>
  );
}

export default App;
