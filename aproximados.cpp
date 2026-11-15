// aproximados.cpp - Persona C: algoritmos aproximados (MST+DFS) y heurístico (Nearest Neighbor)
#include "routeopt.h"
#include <chrono>
#include <limits>

// ---------------- Nearest Neighbor ----------------

std::string NearestNeighborSolver::nombre() const {
    return "Nearest Neighbor";
}

Result NearestNeighborSolver::resolver(const Graph& g, double limiteSeg) {
    auto inicio = std::chrono::steady_clock::now();
    int n = g.n();
    std::vector<bool> visitado(n, false);
    Result r;
    r.algoritmo = nombre();

    int actual = 0;                 // se empieza en el centro
    visitado[0] = true;
    r.ruta.push_back(0);
    long long estados = 0;

    for (int paso = 1; paso < n; paso++) {
        int mejor = -1;
        double mejorDist = std::numeric_limits<double>::infinity();
        for (int c = 1; c < n; c++) {          // buscar el cliente no visitado más cercano
            estados++;
            if (!visitado[c] && g.w[actual][c] < mejorDist) {
                mejorDist = g.w[actual][c];
                mejor = c;
            }
        }
        visitado[mejor] = true;
        r.ruta.push_back(mejor);
        actual = mejor;
    }
    r.ruta.push_back(0);            // regreso al centro

    r.costo = costoRuta(g, r.ruta);
    r.estados = estados;
    r.terminoPorTiempo = false;
    r.tiempoMs = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - inicio).count();
    r.memoriaBytes = (double)n * (sizeof(bool) + sizeof(int));
    return r;
}

// ---------------- MST (Prim) + DFS ----------------

namespace {
// Recorrido en profundidad del árbol: anota cada nodo la primera vez que lo visita.
void dfs(int u, const std::vector<std::vector<int>>& hijos, std::vector<int>& orden, long long& estados) {
    estados++;
    orden.push_back(u);
    for (int v : hijos[u]) dfs(v, hijos, orden, estados);
}
}

std::string MstDfsSolver::nombre() const {
    return "MST + DFS";
}

Result MstDfsSolver::resolver(const Graph& g, double limiteSeg) {
    auto inicio = std::chrono::steady_clock::now();
    int n = g.n();
    const double INF = std::numeric_limits<double>::infinity();
    long long estados = 0;

    // --- Prim: construir el MST empezando en el centro ---
    std::vector<bool> enArbol(n, false);
    std::vector<double> dist(n, INF);     // dist[v] = arista más barata para unir v al árbol
    std::vector<int> padre(n, -1);        // padre[v] = nodo del árbol al que se une v
    dist[0] = 0;
    for (int it = 0; it < n; it++) {
        int u = -1;
        for (int v = 0; v < n; v++) {      // elegir el nodo fuera del árbol con menor dist
            estados++;
            if (!enArbol[v] && (u == -1 || dist[v] < dist[u])) u = v;
        }
        enArbol[u] = true;
        for (int v = 0; v < n; v++) {      // actualizar las aristas desde u
            estados++;
            if (!enArbol[v] && g.w[u][v] < dist[v]) {
                dist[v] = g.w[u][v];
                padre[v] = u;
            }
        }
    }

    // Lista de hijos de cada nodo (así el árbol queda listo para el DFS)
    std::vector<std::vector<int>> hijos(n);
    for (int v = 1; v < n; v++) hijos[padre[v]].push_back(v);

    // --- DFS en preorden desde el centro ---
    Result r;
    r.algoritmo = nombre();
    dfs(0, hijos, r.ruta, estados);
    r.ruta.push_back(0);                   // regreso al centro

    r.costo = costoRuta(g, r.ruta);
    r.estados = estados;
    r.terminoPorTiempo = false;
    r.tiempoMs = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - inicio).count();
    // Memoria estimada: listas dist/padre/enArbol más las listas de hijos
    r.memoriaBytes = (double)n * (sizeof(double) + 2 * sizeof(int) + sizeof(bool)) + (double)n * sizeof(int);
    return r;
}