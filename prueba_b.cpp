#include "routeopt.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <string>

using namespace std;

// Compara dos costos con tolerancia.
bool casiIgual(double a, double b) {
    return fabs(a - b) < 0.000001;
}

// Muestra el resultado de una prueba.
void mostrar(string nombre, bool pasa) {
    cout << nombre << ": "
         << (pasa ? "OK" : "FALLA") << "\n";
}

// Comprueba que la ruta visite todos los nodos
// exactamente una vez y regrese al centro.
bool rutaValida(const Graph& g, const vector<int>& ruta) {

    int n = g.n();

    if ((int)ruta.size() != n + 1)
        return false;

    if (ruta.front() != 0 || ruta.back() != 0)
        return false;

    vector<bool> visitado(n, false);

    for (int i = 0; i < n; i++) {

        int nodo = ruta[i];

        if (nodo < 0 || nodo >= n)
            return false;

        if (visitado[nodo])
            return false;

        visitado[nodo] = true;
    }

    for (int i = 0; i < n; i++) {
        if (!visitado[i])
            return false;
    }

    return true;
}

// Imprime una ruta.
void mostrarRuta(const vector<int>& ruta) {

    for (size_t i = 0; i < ruta.size(); i++) {

        cout << ruta[i];

        if (i + 1 < ruta.size())
            cout << " -> ";
    }

    cout << "\n";
}

int main() {

    cout << "========================================\n";
    cout << " PRUEBAS PERSONA B - ROUTEOPT\n";
    cout << " BACKTRACKING Y HELD-KARP\n";
    cout << "========================================\n";

    BacktrackingSolver backtracking;
    HeldKarpSolver heldKarp;

    // PRUEBA 1: INSTANCIA CONOCIDA

    cout << "\n--- PRUEBA 1: EJEMPLO DE 5 NODOS ---\n";

    Graph g;

    g.nodos = {
        {0, "Centro", 0, 0},
        {1, "C1", 2, 3},
        {2, "C2", 5, 1},
        {3, "C3", 6, 4},
        {4, "C4", 1, 6}
    };

    construirMatriz(g);

    Result rb = backtracking.resolver(g, 60);
    Result rh = heldKarp.resolver(g, 60);

    cout << "\nBACKTRACKING\n";
    cout << "Ruta: ";
    mostrarRuta(rb.ruta);
    cout << "Costo: " << rb.costo << "\n";
    cout << "Estados: " << rb.estados << "\n";
    cout << "Tiempo (ms): " << rb.tiempoMs << "\n";
    cout << "Memoria (bytes): " << rb.memoriaBytes << "\n";

    cout << "\nHELD-KARP\n";
    cout << "Ruta: ";
    mostrarRuta(rh.ruta);
    cout << "Costo: " << rh.costo << "\n";
    cout << "Estados: " << rh.estados << "\n";
    cout << "Tiempo (ms): " << rh.tiempoMs << "\n";
    cout << "Memoria (bytes): " << rh.memoriaBytes << "\n";

    cout << "\nVALIDACIONES\n";

    mostrar(
        "Ruta valida en Backtracking",
        rutaValida(g, rb.ruta)
    );

    mostrar(
        "Ruta valida en Held-Karp",
        rutaValida(g, rh.ruta)
    );

    mostrar(
        "Ambos obtienen el mismo costo",
        casiIgual(rb.costo, rh.costo)
    );

    mostrar(
        "Costo optimo esperado",
        fabs(rb.costo - 20.414) < 0.001
    );

    mostrar(
        "Backtracking termina sin limite",
        !rb.terminoPorTiempo
    );

    mostrar(
        "Held-Karp termina sin limite",
        !rh.terminoPorTiempo
    );

    // PRUEBA 2: INSTANCIAS DE HASTA 12 NODOS

    cout << "\n--- PRUEBA 2: INSTANCIAS ALEATORIAS ---\n";

    bool todasCorrectas = true;

    // Desde 2 hasta 11 clientes:
    // total de 3 hasta 12 nodos incluyendo el centro.
    for (int clientes = 2; clientes <= 11; clientes++) {

        Graph instancia =
            generarInstancia(clientes, 100 + clientes);

        Result b = backtracking.resolver(instancia, 60);
        Result h = heldKarp.resolver(instancia, 60);

        bool mismoCosto = casiIgual(b.costo, h.costo);

        bool rutasCorrectas =
            rutaValida(instancia, b.ruta) &&
            rutaValida(instancia, h.ruta);

        bool terminoCorrectamente =
            !b.terminoPorTiempo &&
            !h.terminoPorTiempo;

        bool correcto =
            mismoCosto &&
            rutasCorrectas &&
            terminoCorrectamente;

        cout << "\nClientes: " << clientes;
        cout << " | Total de nodos: " << instancia.n() << "\n";

        cout << "Costo Backtracking: " << b.costo << "\n";
        cout << "Costo Held-Karp: " << h.costo << "\n";

        mostrar("Mismo costo optimo", mismoCosto);
        mostrar("Rutas validas", rutasCorrectas);
        mostrar("Ejecucion completa", terminoCorrectamente);
        mostrar("Resultado general", correcto);

        if (!correcto)
            todasCorrectas = false;
    }

    // RESULTADO FINAL

    cout << "\n========================================\n";

    mostrar(
        "TODAS LAS PRUEBAS DE PERSONA B",
        todasCorrectas
    );

    cout << "========================================\n";

    return todasCorrectas ? 0 : 1;
}

