#include "routeopt.h"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <limits>
#include <chrono>
#include <algorithm>
#include <functional>
#include <cstdint>
#include <new>

using namespace std;

namespace {

bool grafoValido(const Graph& g) {
    int n = g.n();

    if (n < 2 || (int)g.w.size() != n)
        return false;

    for (int i = 0; i < n; i++) {
        if ((int)g.w[i].size() != n)
            return false;

        for (int j = 0; j < n; j++) {
            if (!isfinite(g.w[i][j]) || g.w[i][j] < 0)
                return false;
        }
    }

    return true;
}

// calcula el costo total de una ruta.
double calcularCosto(const Graph& g, const vector<int>& ruta) {
    double total = 0;

    for (size_t i = 0; i + 1 < ruta.size(); i++)
        total += g.w[ruta[i]][ruta[i + 1]];

    return total;
}

// genera una solucion inicial mediante vecino más cercano
vector<int> generarRutaInicial(const Graph& g) {
    int n = g.n();

    vector<int> ruta;
    vector<bool> visitado(n, false);

    int actual = 0;
    visitado[0] = true;
    ruta.push_back(0);

    for (int i = 1; i < n; i++) {
        int siguiente = -1;
        double menor = numeric_limits<double>::infinity();

        for (int j = 1; j < n; j++) {
            if (!visitado[j] && g.w[actual][j] < menor) {
                menor = g.w[actual][j];
                siguiente = j;
            }
        }

        if (siguiente == -1)
            return {};

        visitado[siguiente] = true;
        ruta.push_back(siguiente);
        actual = siguiente;
    }

    ruta.push_back(0);
    return ruta;
}

//comprobar si alcanzo el limite de tiempo
bool limiteAlcanzado(
    chrono::steady_clock::time_point inicio,
    double limiteSeg
) {
    if (limiteSeg <= 0)
        return false;

    chrono::duration<double> transcurrido =
        chrono::steady_clock::now() - inicio;

    return transcurrido.count() >= limiteSeg;
}

// crea un resultado inicial con una ruta factible
Result crearResultadoInicial(const Graph& g, const string& nombre) {
    Result r;
    r.algoritmo = nombre;

    if (grafoValido(g)) {
        r.ruta = generarRutaInicial(g);
        r.costo = calcularCosto(g, r.ruta);
    } else {
        r.costo = numeric_limits<double>::infinity();
    }

    return r;
}

} 

string BacktrackingSolver::nombre() const {
    return "Backtracking con poda";
}

Result BacktrackingSolver::resolver(
    const Graph& g,
    double limiteSeg
) {
    auto inicio = chrono::steady_clock::now();

    Result resultado = crearResultadoInicial(g, nombre());

    if (!grafoValido(g))
        return resultado;

    int n = g.n();

    double mejorCosto = resultado.costo;
    vector<int> mejorRuta = resultado.ruta;

    vector<int> ruta;
    vector<bool> visitado(n, false);

    ruta.push_back(0);
    visitado[0] = true;

    long long estados = 0;
    bool tiempoAgotado = false;

    const double EPS = 1e-9;

    function<void(int, double)> buscar =
        [&](int actual, double costoActual) {

        estados++;

        if ((estados & 1023) == 0 && limiteAlcanzado(inicio, limiteSeg)) {
            tiempoAgotado = true;
            return;
        }

        if ((int)ruta.size() == n) {

            double costoTotal = costoActual + g.w[actual][0];

            if (costoTotal < mejorCosto - EPS) {
                mejorCosto = costoTotal;
                mejorRuta = ruta;
                mejorRuta.push_back(0);
            }

            return;
        }

        for (int siguiente = 1; siguiente < n; siguiente++) {

            if (tiempoAgotado)
                return;

            if (!visitado[siguiente]) {

                double nuevoCosto =
                    costoActual + g.w[actual][siguiente];

                if (nuevoCosto >= mejorCosto - EPS)
                    continue;

                visitado[siguiente] = true;
                ruta.push_back(siguiente);

                buscar(siguiente, nuevoCosto);

                ruta.pop_back();
                visitado[siguiente] = false;
            }
        }
    };

    buscar(0, 0.0);

    resultado.ruta = mejorRuta;
    resultado.costo = mejorCosto;
    resultado.estados = estados;
    resultado.terminoPorTiempo = tiempoAgotado;

    auto fin = chrono::steady_clock::now();

    resultado.tiempoMs =
        chrono::duration<double, milli>(fin - inicio).count();

    resultado.memoriaBytes =
        2 * n * sizeof(int) +
        n * sizeof(bool) +
        sizeof(estados);

    return resultado;
}

string HeldKarpSolver::nombre() const {
    return "Held-Karp";
}

Result HeldKarpSolver::resolver(
    const Graph& g,
    double limiteSeg
) {
    auto inicio = chrono::steady_clock::now();

    Result resultado = crearResultadoInicial(g, nombre());

    if (!grafoValido(g))
        return resultado;

    int n = g.n();
    int m = n - 1; // cantidad de clientes

    if (m > 20) {
        resultado.terminoPorTiempo = true;
        return resultado;
    }

    size_t totalMasks = size_t(1) << m;

    size_t cantidad = totalMasks * (size_t)m;

    const double INF = numeric_limits<double>::infinity();
    const double EPS = 1e-9;

    vector<double> dp;
    vector<int> padre;

    try {
        // tabla de costos minimos
        dp.assign(cantidad, INF);

        // tabla para reconstruir la ruta
        padre.assign(cantidad, -1);

    } catch (const bad_alloc&) {
        resultado.terminoPorTiempo = true;
        return resultado;
    }

    auto indice = [m](size_t mask, int j) {
        return mask * (size_t)m + j;
    };

    long long estados = 0;
    bool tiempoAgotado = false;

    for (int j = 0; j < m; j++) {

        if (limiteAlcanzado(inicio, limiteSeg)) {
            tiempoAgotado = true;
            break;
        }

        size_t mask = size_t(1) << j;

        dp[indice(mask, j)] = g.w[0][j + 1];

        estados++;
    }

    // transiciones

    for (size_t mask = 1;
         mask < totalMasks && !tiempoAgotado;
         mask++) {

        for (int j = 0; j < m; j++) {

            if (!(mask & (size_t(1) << j)))
                continue;

            double actual = dp[indice(mask, j)];

            if (!isfinite(actual))
                continue;

            estados++;

            for (int k = 0; k < m; k++) {

                if (mask & (size_t(1) << k))
                    continue;
                    
                if ((estados & 1023) == 0 && limiteAlcanzado(inicio, limiteSeg)) {
                    tiempoAgotado = true;
                    break;
                }
                size_t nuevaMask =
                    mask | (size_t(1) << k);

                double nuevoCosto =
                    actual + g.w[j + 1][k + 1];

                size_t pos = indice(nuevaMask, k);

                if (nuevoCosto < dp[pos] - EPS) {

                    dp[pos] = nuevoCosto;

                    padre[pos] = j;
                }
            }

            if (tiempoAgotado)
                break;
        }
    }

    size_t mascaraCompleta = totalMasks - 1;

    double mejorCosto = INF;
    int ultimo = -1;

    for (int j = 0; j < m; j++) {

        double costo = dp[indice(mascaraCompleta, j)];

        if (!isfinite(costo))
            continue;

        costo += g.w[j + 1][0];

        if (costo < mejorCosto - EPS) {
            mejorCosto = costo;
            ultimo = j;
        }
    }

    // reconstruir la ruta

    if (ultimo != -1 &&
        mejorCosto < resultado.costo - EPS) {

        vector<int> rutaInvertida;

        size_t mask = mascaraCompleta;
        int actual = ultimo;

        // reconstrucción utilizando la tabla de padres
        while (actual != -1) {

            rutaInvertida.push_back(actual + 1);

            int anterior = padre[indice(mask, actual)];

            mask ^= (size_t(1) << actual);

            actual = anterior;
        }

        // invertir para obtener el recorrido correcto
        reverse(rutaInvertida.begin(), rutaInvertida.end());

        vector<int> rutaFinal;

        rutaFinal.push_back(0);

        for (int cliente : rutaInvertida)
            rutaFinal.push_back(cliente);

        rutaFinal.push_back(0);

        resultado.ruta = rutaFinal;
        resultado.costo = mejorCosto;
    }

    // metricas

    resultado.estados = estados;
    resultado.terminoPorTiempo = tiempoAgotado;

    auto fin = chrono::steady_clock::now();

    resultado.tiempoMs =
        chrono::duration<double, milli>(fin - inicio).count();

    // memoria estimada de las tablas
    resultado.memoriaBytes =
        cantidad * sizeof(double) +
        cantidad * sizeof(int);

    return resultado;
}
