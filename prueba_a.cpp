// prueba_a.cpp - pruebas - Persona A
#include "routeopt.h"
#include <iostream>
#include <cmath>
#include <fstream>

bool casi(double a, double b) { return std::fabs(a - b) < 0.01; }

void mostrar(const std::string& nombre, bool pasa) {
    std::cout << nombre << ": " << (pasa ? "OK" : "FALLA") << "\n";
}

int main() {
    // Ejemplo del Avance 1: centro D y 4 clientes
    Graph g;
    g.nodos = { {0,"Centro",0,0}, {1,"C1",2,3}, {2,"C2",5,1},
                {3,"C3",6,4}, {4,"C4",1,6} };
    construirMatriz(g);

    std::cout << "w[0][1] = " << g.w[0][1] << "  (esperado 3.61)\n";
    std::cout << (casi(g.w[0][1], 3.61) ? "OK" : "FALLA") << "\n\n";

    std::vector<int> optima = {0, 1, 4, 3, 2, 0};
    double c1 = costoRuta(g, optima);
    std::cout << "costo optima = " << c1 << "  (esperado 20.41)\n";
    std::cout << (casi(c1, 20.41) ? "OK" : "FALLA") << "\n\n";

    std::vector<int> otra = {0, 2, 3, 1, 4, 0};
    double c2 = costoRuta(g, otra);
    std::cout << "costo otra ruta = " << c2 << "  (esperado 21.63)\n";
    std::cout << (casi(c2, 21.63) ? "OK" : "FALLA") << "\n";

        // ---------- Pruebas de generarInstancia ----------
    std::cout << "\n--- generarInstancia ---\n";
    Graph a1 = generarInstancia(10, 42);
    Graph a2 = generarInstancia(10, 42);
    Graph a3 = generarInstancia(10, 43);
    bool igual = true;
    for (int i = 0; i < a1.n(); i++)
        if (a1.nodos[i].x != a2.nodos[i].x || a1.nodos[i].y != a2.nodos[i].y) igual = false;
    mostrar("tiene 11 nodos (centro + 10)", a1.n() == 11);
    mostrar("misma semilla da la misma instancia", igual);
    mostrar("semilla distinta da otra instancia", a1.nodos[0].x != a3.nodos[0].x);

    // PRUEBAS DE LA FUNCION VALIDAR
    std::cout << "\n--- validar ---\n";
    std::string err;
    mostrar("acepta instancia aleatoria (con triangular)", validar(a1, true, err));
    mostrar("acepta el ejemplo de 5 nodos", validar(g, true, err));

    Graph malo = g;
    malo.w[1][2] = -3;
    mostrar("rechaza peso negativo", !validar(malo, false, err));
    std::cout << "   motivo: " << err << "\n";

    malo = g;
    malo.w[1][2] += 5;   // w[2][1] queda igual -> asimetria
    mostrar("rechaza matriz no simetrica", !validar(malo, false, err));
    std::cout << "   motivo: " << err << "\n";

    malo = g;
    malo.w[3][3] = 2;
    mostrar("rechaza diagonal distinta de 0", !validar(malo, false, err));
    std::cout << "   motivo: " << err << "\n";

    malo = g;
    malo.w[0].pop_back();   // quita una columna
    mostrar("rechaza matriz no cuadrada", !validar(malo, false, err));
    std::cout << "   motivo: " << err << "\n";

    // Matriz simetrica pero que rompe la desigualdad triangular:
    // ir de 0 a 2 directo cuesta 5, pero pasando por 1 cuesta 1 + 1 = 2
    Graph t;
    t.nodos = { {0,"A",0,0}, {1,"B",0,0}, {2,"C",0,0} };
    t.w = { {0,1,5}, {1,0,1}, {5,1,0} };
    mostrar("sin revisar triangular, la acepta", validar(t, false, err));
    mostrar("revisando triangular, la rechaza", !validar(t, true, err));
    std::cout << "   motivo: " << err << "\n";

    // Pruebas de cargarCSV y exportarCSV
    // Cargar CSV
    std::cout << "\n--- cargarCSV ---\n";
    {   // las llaves cierran el archivo al terminar, para que quede guardado
        std::ofstream f("instancia_prueba.csv");
        f << "id,nombre,x,y\n";
        f << "0,Centro,0,0\n1,C1,2,3\n2,C2,5,1\n3,C3,6,4\n4,C4,1,6\n";
    }
    Graph cg = cargarCSV("instancia_prueba.csv");
    mostrar("carga 5 nodos", cg.n() == 5);
    mostrar("costo de la ruta optima coincide (20.41)", casi(costoRuta(cg, optima), 20.41));
    mostrar("el CSV cargado pasa validar", validar(cg, true, err));

    Graph vacio = cargarCSV("no_existe.csv");
    mostrar("archivo inexistente da grafo vacio", vacio.n() == 0);
    mostrar("validar rechaza el grafo vacio", !validar(vacio, false, err));
    std::cout << "   motivo: " << err << "\n";

    {
        std::ofstream f("instancia_mala.csv");
        f << "id,nombre,x,y\n0,Centro,0,0\n1,C1,abc,3\n";
    }
    mostrar("fila con texto en vez de numero da grafo vacio", cargarCSV("instancia_mala.csv").n() == 0);

    // Exportar CSV
    std::cout << "\n--- exportarCSV ---\n";
    Result r1; r1.algoritmo = "Prueba1"; r1.ruta = optima; r1.costo = c1; r1.estados = 100;
    Result r2; r2.algoritmo = "Prueba2"; r2.ruta = otra;   r2.costo = c2; r2.desviacion = 5.95;
    exportarCSV("resultados_prueba.csv", {r1, r2});

    std::ifstream in("resultados_prueba.csv");
    std::string linea;
    int lineas = 0;
    while (std::getline(in, linea)) {
        std::cout << "   " << linea << "\n";
        lineas++;
    }
    mostrar("el archivo tiene encabezado + 2 filas", lineas == 3);

    return 0;
}