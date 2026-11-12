// datos.cpp
// Persona A: datos y grafo

#include "routeopt.h"
#include <cmath>
#include <random>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

// Llena la matriz w con la distancia euclidiana entre cada par de nodos.
// La diagonal queda en 0 (w[i][i] = 0), como se pide en el modelo del Avance 1.
void construirMatriz(Graph& g) {
    int n = g.n();
    g.w.assign(n, std::vector<double>(n, 0.0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i != j) {
                double dx = g.nodos[i].x - g.nodos[j].x;
                double dy = g.nodos[i].y - g.nodos[j].y;
                g.w[i][j] = std::sqrt(dx * dx + dy * dy);
            }
        }
    }
}

// Suma la distancia de cada paso consecutivo de la ruta.
// OJO: La ruta incluye el regreso al centro: (0, a, b, ..., 0)
double costoRuta(const Graph& g, const std::vector<int>& ruta) {
    double total = 0;
    for (size_t k = 0; k + 1 < ruta.size(); k++) {
        total += g.w[ruta[k]][ruta[k + 1]];
    }
    return total;
}

// Crea una instancia aleatoria: 1 centro + 'clientes' clientes,
// con coordenadas entre 0 y 1000. La misma semilla da siempre la misma instancia.
Graph generarInstancia(int clientes, unsigned semilla) {
    std::mt19937 rng(semilla);                                   // generador de números aleatorios
    std::uniform_real_distribution<double> coord(0.0, 1000.0);   // números entre 0 y 1000
    Graph g;
    for (int i = 0; i <= clientes; i++) {
        Node nd;
        nd.id = i;
        if (i == 0) nd.nombre = "Centro";
        else        nd.nombre = "C" + std::to_string(i);
        nd.x = coord(rng);
        nd.y = coord(rng);
        g.nodos.push_back(nd);
    }
    construirMatriz(g);
    return g;
}

// Revisa que la instancia sea correcta. Si encuentra un problema, escribe
// el motivo en 'error' y devuelve false. Si todo está bien, devuelve true.
bool validar(const Graph& g, bool revisarTriangular, std::string& error) {
    int n = g.n();
    if (n < 2) {
        error = "Se necesita el centro y al menos un cliente";
        return false;
    }
    if ((int)g.w.size() != n) {
        error = "La matriz no tiene una fila por nodo";
        return false;
    }
    for (int i = 0; i < n; i++) {
        if ((int)g.w[i].size() != n) {
            error = "La matriz no es cuadrada (fila " + std::to_string(i) + ")";
            return false;
        }
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double v = g.w[i][j];
            if (!(v >= 0)) {   // atrapa negativos y valores inválidos (NaN)
                error = "Peso negativo o invalido en (" + std::to_string(i) + "," + std::to_string(j) + ")";
                return false;
            }
            if (i == j && v != 0) {
                error = "La diagonal debe ser 0 (nodo " + std::to_string(i) + ")";
                return false;
            }
            if (std::fabs(v - g.w[j][i]) > 1e-9) {
                error = "Matriz no simetrica en (" + std::to_string(i) + "," + std::to_string(j) + ")";
                return false;
            }
        }
    }
    if (revisarTriangular) {   // ir directo nunca debe costar más que dar un rodeo
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                for (int k = 0; k < n; k++)
                    if (g.w[i][k] > g.w[i][j] + g.w[j][k] + 1e-9) {
                        error = "No cumple la desigualdad triangular";
                        return false;
                    }
    }
    return true;
}

// Lee un archivo CSV con columnas id,nombre,x,y y arma el grafo.
// La primera fila de datos es el centro. Si el archivo no existe o tiene una fila mal escrita, devuelve un grafo vacío (n() == 0); luego 'validar' lo rechaza.
Graph cargarCSV(const std::string& archivo) {
    Graph g;
    std::ifstream f(archivo);                 // abre el archivo para leer
    if (!f.is_open()) return g;               // no se pudo abrir: grafo vacío

    std::string linea;
    std::getline(f, linea);                   // primera línea = encabezado, se descarta
    while (std::getline(f, linea)) {          // lee una línea a la vez
        if (!linea.empty() && linea.back() == '\r') linea.pop_back();  // quita salto de línea de Windows
        if (linea.empty()) continue;          // ignora líneas vacías

        std::stringstream ss(linea);          // permite cortar la línea por comas
        std::string campo[4];
        for (int c = 0; c < 4; c++) {
            if (!std::getline(ss, campo[c], ',')) return Graph();   // faltan columnas
        }

        Node nd;
        try {
            nd.x = std::stod(campo[2]);       // texto -> número decimal
            nd.y = std::stod(campo[3]);
        } catch (...) {
            return Graph();                   // no era un número
        }
        nd.id = (int)g.nodos.size();          // el id es la posición (0 = centro)
        nd.nombre = campo[1];
        g.nodos.push_back(nd);
    }
    construirMatriz(g);
    return g;
}

// Guarda los resultados de los algoritmos en un archivo CSV, una fila por resultado.
// La ruta se escribe con espacios (por ejemplo "0 1 4 3 2 0") para no mezclarla con las comas.
void exportarCSV(const std::string& archivo, const std::vector<Result>& res) {
    std::ofstream f(archivo);                 // abre (o crea) el archivo para escribir
    if (!f.is_open()) {
        std::cerr << "No se pudo crear " << archivo << "\n";
        return;
    }
    f << std::fixed << std::setprecision(4);  // 4 decimales, sin notación científica
    f << "algoritmo,costo,tiempoMs,memoriaBytes,estados,desviacion,terminoPorTiempo,ruta\n";
    for (const Result& r : res) {
        f << r.algoritmo << "," << r.costo << "," << r.tiempoMs << ","
          << r.memoriaBytes << "," << r.estados << "," << r.desviacion << ","
          << (r.terminoPorTiempo ? 1 : 0) << ",";
        for (size_t k = 0; k < r.ruta.size(); k++) {
            if (k > 0) f << " ";
            f << r.ruta[k];
        }
        f << "\n";
    }
}