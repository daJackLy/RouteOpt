#pragma once
#include <string>
#include <vector>

// CONTRATO COMUN

struct Node {
    int id;               // 0 = centro de distribución
    std::string nombre;
    double x, y;
};

struct Graph {
    std::vector<Node> nodos;              // nodos[0] es el centro
    std::vector<std::vector<double>> w;   // matriz de distancias
    int n() const { return (int)nodos.size(); }   // total de nodos (clientes + 1)
};

struct Result {
    std::string algoritmo;
    std::vector<int> ruta;       // incluye el regreso: 0, a, b, c, ..., 0
    double costo = 0;
    double tiempoMs = 0;
    double memoriaBytes = 0;     // estimada
    long long estados = 0;       // nodos explorados u operaciones
    double desviacion = 0;       // % respecto al óptimo (la calcula quien corre el experimento)
    bool terminoPorTiempo = false;
};

class ITspSolver {
public:
    virtual ~ITspSolver() = default;
    virtual std::string nombre() const = 0;
    virtual Result resolver(const Graph& g, double limiteSeg) = 0;
};

// FUNCIONES DE A (Jhair Cardenas) (Implementdos en datos.cpp)

Graph cargarCSV(const std::string& archivo);               // lee nodos y arma la matriz
Graph generarInstancia(int clientes, unsigned semilla);    // aleatoria y reproducible
void construirMatriz(Graph& g);                            // llena g.w con distancias euclidianas
bool validar(const Graph& g, bool revisarTriangular, std::string& error);
double costoRuta(const Graph& g, const std::vector<int>& ruta);
void exportarCSV(const std::string& archivo, const std::vector<Result>& res);

// CLASES DE LOS ALGORITMOS (cada método se implementa en su .cpp)

// Persona B -> exactos.cpp
class BacktrackingSolver : public ITspSolver {
public:
    std::string nombre() const override;
    Result resolver(const Graph& g, double limiteSeg) override;
};
class HeldKarpSolver : public ITspSolver {
public:
    std::string nombre() const override;
    Result resolver(const Graph& g, double limiteSeg) override;
};

// Persona C -> aproximados.cpp
class MstDfsSolver : public ITspSolver {
public:
    std::string nombre() const override;
    Result resolver(const Graph& g, double limiteSeg) override;
};
class NearestNeighborSolver : public ITspSolver {
public:
    std::string nombre() const override;
    Result resolver(const Graph& g, double limiteSeg) override;
};