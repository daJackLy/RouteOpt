// main.cpp - Persona C: menú, piloto y tabla comparativa
#include "routeopt.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <map>
#include <string>

// Desviación porcentual respecto a una referencia (óptimo o mejor solución conocida)
double desviacion(double costo, double referencia) {
    return (costo - referencia) / referencia * 100.0;
}

// Ejecuta los algoritmos que corresponden al tamaño de la instancia y calcula
// la desviación de cada uno. Backtracking hasta 12 clientes, Held-Karp hasta 18,
// y los dos aproximados siempre.
std::vector<Result> compararEnInstancia(const Graph& g) {
    int clientes = g.n() - 1;
    std::vector<Result> res;

    if (clientes <= 12) { BacktrackingSolver bt; res.push_back(bt.resolver(g, 30.0)); }
    if (clientes <= 18) { HeldKarpSolver hk;     res.push_back(hk.resolver(g, 60.0)); }
    MstDfsSolver mst;         res.push_back(mst.resolver(g, 60.0));
    NearestNeighborSolver nn; res.push_back(nn.resolver(g, 60.0));

    // Referencia: el menor costo de un exacto que terminó (el óptimo).
    // Si no hubo exactos, la mejor solución conocida entre todos.
    double ref = std::numeric_limits<double>::infinity();
    bool hayExacto = false;
    for (const Result& r : res) {
        bool esExacto = (r.algoritmo == "Backtracking con poda" || r.algoritmo == "Held-Karp");
        if (esExacto && !r.terminoPorTiempo && r.costo < ref) { ref = r.costo; hayExacto = true; }
    }
    if (!hayExacto)
        for (const Result& r : res) if (r.costo < ref) ref = r.costo;

    for (Result& r : res) r.desviacion = desviacion(r.costo, ref);
    return res;
}

// Muestra un resultado en pantalla
void imprimirResultado(const Result& r) {
    std::cout << "\n" << r.algoritmo << "\n";
    std::cout << "  costo = " << r.costo << " | tiempo = " << r.tiempoMs << " ms"
              << " | estados = " << r.estados
              << " | memoria = " << r.memoriaBytes / 1024.0 << " KB"
              << " | desviacion = " << r.desviacion << " %\n";
    if (r.terminoPorTiempo) std::cout << "  (se corto por limite de tiempo)\n";
    std::cout << "  ruta: ";
    for (size_t k = 0; k < r.ruta.size() && k < 40; k++) std::cout << r.ruta[k] << " ";
    if (r.ruta.size() > 40) std::cout << "...";
    std::cout << "\n";
}

// Acumulador para promediar los resultados de un algoritmo en un tamaño
struct Acum {
    double costo = 0, tiempo = 0, memoria = 0, desv = 0;
    long long estados = 0;
    int n = 0, cortados = 0;
};

// Simulación piloto: tabla de promedios por tamaño y exportación a CSV
void piloto() {
    const int INSTANCIAS = 5;      // instancias por tamaño (en el informe final usaremos 10)
    int tamanos[] = {5, 8, 10, 12, 15, 18, 50, 200, 1000};
    std::vector<Result> todos;     // todos los resultados individuales, para exportar

    std::cout << "\n=== PILOTO: promedio de " << INSTANCIAS
              << " instancias por tamano (coordenadas en [0,1000]^2) ===\n";

    for (int cl : tamanos) {
        std::map<std::string, Acum> acum;
        for (int s = 1; s <= INSTANCIAS; s++) {
            Graph g = generarInstancia(cl, 1000 * cl + s);   // semilla fija -> reproducible
            std::vector<Result> res = compararEnInstancia(g);
            for (Result& r : res) {
                Acum& a = acum[r.algoritmo];
                a.costo += r.costo;
                a.tiempo += r.tiempoMs;
                a.memoria += r.memoriaBytes;
                a.desv += r.desviacion;
                a.estados += r.estados;
                a.n++;
                if (r.terminoPorTiempo) a.cortados++;
                // para el CSV, el nombre incluye tamaño y semilla
                r.algoritmo = "n=" + std::to_string(cl) + " semilla=" + std::to_string(s) + " " + r.algoritmo;
                todos.push_back(r);
            }
        }

        std::cout << "\n" << cl << " clientes";
        if (cl > 18) std::cout << "  (solo aproximados; desviacion respecto a la mejor solucion conocida)\n";
        else         std::cout << "  (desviacion respecto al optimo)\n";
        std::cout << std::left << std::setw(24) << "Algoritmo" << std::right
                  << std::setw(12) << "Costo" << std::setw(12) << "Tiempo(ms)"
                  << std::setw(14) << "Estados" << std::setw(12) << "Mem(KB)"
                  << std::setw(10) << "Desv(%)" << "\n";
        for (const auto& par : acum) {
            const Acum& a = par.second;
            std::cout << std::left << std::setw(24) << par.first << std::right
                      << std::fixed << std::setprecision(2)
                      << std::setw(12) << a.costo / a.n
                      << std::setw(12) << a.tiempo / a.n
                      << std::setw(14) << (long long)(a.estados / a.n)
                      << std::setw(12) << a.memoria / a.n / 1024.0
                      << std::setw(10) << a.desv / a.n;
            if (a.cortados > 0) std::cout << "  (" << a.cortados << " cortados por tiempo)";
            std::cout << "\n";
        }
    }
    std::cout << std::defaultfloat;

    exportarCSV("resultados_piloto.csv", todos);
    std::cout << "\nResultados individuales exportados a resultados_piloto.csv\n";
}

int main() {
    Graph g;                      // instancia actual
    bool hayInstancia = false;
    int op = -1;

    while (op != 0) {
        std::cout << "\n===== RouteOpt =====\n"
                  << "1) Cargar instancia desde CSV\n"
                  << "2) Generar instancia aleatoria\n"
                  << "3) Ejecutar algoritmos sobre la instancia actual\n"
                  << "4) Piloto (tabla comparativa y exportacion a CSV)\n"
                  << "0) Salir\n"
                  << "Opcion: ";
        if (!(std::cin >> op)) break;

        if (op == 1) {
            std::string archivo;
            std::cout << "Archivo CSV (sin espacios): ";
            std::cin >> archivo;
            Graph nuevo = cargarCSV(archivo);
            std::string err;
            if (validar(nuevo, false, err)) {
                g = nuevo;
                hayInstancia = true;
                std::cout << "Instancia cargada: " << g.n() - 1 << " clientes.\n";
                std::string err2;
                if (g.n() <= 200 && !validar(g, true, err2))
                    std::cout << "Aviso: " << err2 << ". MST+DFS pierde su garantia de aproximacion.\n";
            } else {
                std::cout << "No se pudo usar el archivo: " << err << "\n";
            }
        } else if (op == 2) {
            int cl;
            unsigned semilla;
            std::cout << "Numero de clientes (1 a 1000): ";
            if (!(std::cin >> cl)) break;
            std::cout << "Semilla: ";
            if (!(std::cin >> semilla)) break;
            if (cl < 1 || cl > 1000) {
                std::cout << "Valor fuera de rango.\n";
            } else {
                g = generarInstancia(cl, semilla);
                hayInstancia = true;
                std::cout << "Instancia generada: " << cl << " clientes.\n";
            }
        } else if (op == 3) {
            if (!hayInstancia) {
                std::cout << "Primero cargue o genere una instancia.\n";
            } else {
                std::vector<Result> res = compararEnInstancia(g);
                for (const Result& r : res) imprimirResultado(r);
                exportarCSV("resultados.csv", res);
                std::cout << "\nResultados exportados a resultados.csv\n";
            }
        } else if (op == 4) {
            piloto();
        } else if (op != 0) {
            std::cout << "Opcion no valida.\n";
        }
    }
    return 0;
}