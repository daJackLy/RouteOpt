// prueba_c.cpp - pruebas de la Persona C
#include "routeopt.h"
#include <iostream>
#include <cmath>

void mostrar(const std::string& nombre, bool pasa) {
    std::cout << nombre << ": " << (pasa ? "OK" : "FALLA") << "\n";
}

// Comprueba que la ruta pasa por cada cliente exactamente una vez y empieza y termina en 0
bool rutaValida(const Graph& g, const std::vector<int>& ruta) {
    int n = g.n();
    if ((int)ruta.size() != n + 1) return false;
    if (ruta.front() != 0 || ruta.back() != 0) return false;
    std::vector<int> cuenta(n, 0);
    for (int k = 0; k < n; k++) cuenta[ruta[k]]++;      // sin contar el regreso
    for (int i = 0; i < n; i++) if (cuenta[i] != 1) return false;
    return true;
}

int main() {
    Graph g;
    g.nodos = { {0,"Centro",0,0}, {1,"C1",2,3}, {2,"C2",5,1},
                {3,"C3",6,4}, {4,"C4",1,6} };
    construirMatriz(g);

    NearestNeighborSolver nn;
    MstDfsSolver mst;
    HeldKarpSolver hk;

    std::cout << "--- Ejemplo de 5 nodos (optimo 20.41) ---\n";
    Result rn = nn.resolver(g, 10.0);
    Result rm = mst.resolver(g, 10.0);
    std::cout << "NN:  costo = " << rn.costo << "\n";
    std::cout << "MST: costo = " << rm.costo << "\n";
    mostrar("NN devuelve ruta valida", rutaValida(g, rn.ruta));
    mostrar("MST+DFS devuelve ruta valida", rutaValida(g, rm.ruta));
    mostrar("NN no mejora al optimo", rn.costo >= 20.41 - 0.01);
    mostrar("MST+DFS cumple costo <= 2 * optimo", rm.costo <= 2 * 20.4143 + 1e-9);

    // La prueba acordada: MST+DFS debe cumplir C <= 2 * C*  (C* = optimo de Held-Karp)
    std::cout << "\n--- Garantia C <= 2*C* en 40 instancias euclidianas ---\n";
    bool todasOk = true, todasValidas = true;
    double peorRazon = 0;
    for (int s = 1; s <= 40; s++) {
        int clientes = 5 + (s % 8);                // de 5 a 12 clientes
        Graph gi = generarInstancia(clientes, s);
        double optimo = hk.resolver(gi, 60.0).costo;
        Result a = mst.resolver(gi, 60.0);
        Result b = nn.resolver(gi, 60.0);
        double razon = a.costo / optimo;
        if (razon > peorRazon) peorRazon = razon;
        if (a.costo > 2 * optimo + 1e-9) todasOk = false;
        if (!rutaValida(gi, a.ruta) || !rutaValida(gi, b.ruta)) todasValidas = false;
    }
    std::cout << "peor razon MST+DFS / optimo = " << peorRazon << "\n";
    mostrar("MST+DFS cumple C <= 2*C* en todas", todasOk);
    mostrar("todas las rutas de NN y MST+DFS son validas", todasValidas);

    // Instancia grande: solo los aproximados
    std::cout << "\n--- 1000 clientes ---\n";
    Graph grande = generarInstancia(1000, 9);
    Result gn = nn.resolver(grande, 60.0);
    Result gm = mst.resolver(grande, 60.0);
    std::cout << "NN:  costo = " << gn.costo << ", tiempo = " << gn.tiempoMs << " ms\n";
    std::cout << "MST: costo = " << gm.costo << ", tiempo = " << gm.tiempoMs << " ms\n";
    mostrar("NN ruta valida (1000)", rutaValida(grande, gn.ruta));
    mostrar("MST+DFS ruta valida (1000)", rutaValida(grande, gm.ruta));

    return 0;
}