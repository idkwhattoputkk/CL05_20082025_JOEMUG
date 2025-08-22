#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <pthread.h>
#include <omp.h>
#include <iomanip>

// Estructura para pasar datos a los hilos pthread
struct MatrixThreadData {
    const std::vector<std::vector<int>>* matrix_a;
    const std::vector<std::vector<int>>* matrix_b;
    std::vector<std::vector<int>>* matrix_c;
    int start_row;
    int end_row;
    int n, m, p;
};

// Función para generar matriz con valores aleatorios
std::vector<std::vector<int>> generarMatriz(int rows, int cols) {
    std::vector<std::vector<int>> matrix(rows, std::vector<int>(cols));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = dis(gen);
        }
    }
    return matrix;
}

// Función para imprimir matriz (solo para matrices pequeñas)
void imprimirMatriz(const std::vector<std::vector<int>>& matrix, const std::string& nombre) {
    if (matrix.size() > 10 || matrix[0].size() > 10) {
        std::cout << nombre << " (matriz muy grande, mostrando solo esquina superior izquierda):" << std::endl;
        for (int i = 0; i < std::min(5, static_cast<int>(matrix.size())); ++i) {
            for (int j = 0; j < std::min(5, static_cast<int>(matrix[0].size())); ++j) {
                std::cout << std::setw(4) << matrix[i][j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "..." << std::endl;
    } else {
        std::cout << nombre << ":" << std::endl;
        for (const auto& row : matrix) {
            for (int val : row) {
                std::cout << std::setw(4) << val << " ";
            }
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}

// Función que ejecuta cada hilo pthread para multiplicación de matrices
void* multiplicarFilas(void* arg) {
    MatrixThreadData* data = static_cast<MatrixThreadData*>(arg);
    
    // Cada hilo calcula las filas asignadas de la matriz resultado
    for (int i = data->start_row; i < data->end_row; ++i) {
        for (int j = 0; j < data->p; ++j) {
            (*data->matrix_c)[i][j] = 0;
            for (int k = 0; k < data->m; ++k) {
                (*data->matrix_c)[i][j] += (*data->matrix_a)[i][k] * (*data->matrix_b)[k][j];
            }
        }
    }
    
    return nullptr;
}

// Versión secuencial de multiplicación de matrices
std::vector<std::vector<int>> multiplicarMatricesSecuencial(
    const std::vector<std::vector<int>>& A,
    const std::vector<std::vector<int>>& B) {
    
    int n = A.size();
    int m = A[0].size();
    int p = B[0].size();
    
    std::vector<std::vector<int>> C(n, std::vector<int>(p, 0));
    
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < p; ++j) {
            for (int k = 0; k < m; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    
    return C;
}

// Versión con pthread
std::vector<std::vector<int>> multiplicarMatricesPthread(
    const std::vector<std::vector<int>>& A,
    const std::vector<std::vector<int>>& B,
    int num_threads) {
    
    int n = A.size();
    int m = A[0].size();
    int p = B[0].size();
    
    std::vector<std::vector<int>> C(n, std::vector<int>(p, 0));
    std::vector<pthread_t> threads(num_threads);
    std::vector<MatrixThreadData> thread_data(num_threads);
    
    int chunk_size = n / num_threads;
    
    // Crear y lanzar hilos
    for (int i = 0; i < num_threads; ++i) {
        thread_data[i].matrix_a = &A;
        thread_data[i].matrix_b = &B;
        thread_data[i].matrix_c = &C;
        thread_data[i].start_row = i * chunk_size;
        thread_data[i].end_row = (i == num_threads - 1) ? n : (i + 1) * chunk_size;
        thread_data[i].n = n;
        thread_data[i].m = m;
        thread_data[i].p = p;
        
        pthread_create(&threads[i], nullptr, multiplicarFilas, &thread_data[i]);
    }
    
    // Esperar a que todos los hilos terminen
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
    
    return C;
}

// Versión con OpenMP
std::vector<std::vector<int>> multiplicarMatricesOpenMP(
    const std::vector<std::vector<int>>& A,
    const std::vector<std::vector<int>>& B) {
    
    int n = A.size();
    int m = A[0].size();
    int p = B[0].size();
    
    std::vector<std::vector<int>> C(n, std::vector<int>(p, 0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < p; ++j) {
            C[i][j] = 0;
            for (int k = 0; k < m; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    
    return C;
}

// Función para verificar que dos matrices son iguales
bool matricesIguales(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B) {
    if (A.size() != B.size() || A[0].size() != B[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A[0].size(); ++j) {
            if (A[i][j] != B[i][j]) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    const int N = 1000; // Filas de matriz A
    const int M = 1000; // Columnas de matriz A / Filas de matriz B
    const int P = 1000; // Columnas de matriz B
    const int NUM_THREADS = 8;
    
    std::cout << "=== EJERCICIO 2: MULTIPLICACIÓN DE MATRICES PARALELA ===" << std::endl;
    std::cout << "Matriz A: " << N << " x " << M << std::endl;
    std::cout << "Matriz B: " << M << " x " << P << std::endl;
    std::cout << "Matriz resultado C: " << N << " x " << P << std::endl;
    std::cout << "Número de hilos: " << NUM_THREADS << std::endl;
    std::cout << std::endl;
    
    // Generar matrices
    std::cout << "Generando matriz A...";
    std::cout.flush();
    auto matrix_a = generarMatriz(N, M);
    std::cout << " Completado!" << std::endl;
    
    std::cout << "Generando matriz B...";
    std::cout.flush();
    auto matrix_b = generarMatriz(M, P);
    std::cout << " Completado!" << std::endl;
    
    // Mostrar matrices pequeñas para verificación
    if (N <= 10 && M <= 10 && P <= 10) {
        imprimirMatriz(matrix_a, "Matriz A");
        imprimirMatriz(matrix_b, "Matriz B");
    }
    
    std::vector<std::vector<int>> resultado_secuencial, resultado_pthread, resultado_openmp;
    
    // Medir tiempo de versión secuencial
    std::cout << "Ejecutando multiplicación SECUENCIAL...";
    std::cout.flush();
    auto start = std::chrono::high_resolution_clock::now();
    resultado_secuencial = multiplicarMatricesSecuencial(matrix_a, matrix_b);
    auto end = std::chrono::high_resolution_clock::now();
    auto tiempo_secuencial = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << " Completado!" << std::endl;
    
    // Medir tiempo de versión pthread
    std::cout << "Ejecutando multiplicación PTHREAD...";
    std::cout.flush();
    start = std::chrono::high_resolution_clock::now();
    resultado_pthread = multiplicarMatricesPthread(matrix_a, matrix_b, NUM_THREADS);
    end = std::chrono::high_resolution_clock::now();
    auto tiempo_pthread = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << " Completado!" << std::endl;
    
    // Medir tiempo de versión OpenMP
    std::cout << "Ejecutando multiplicación OPENMP...";
    std::cout.flush();
    start = std::chrono::high_resolution_clock::now();
    resultado_openmp = multiplicarMatricesOpenMP(matrix_a, matrix_b);
    end = std::chrono::high_resolution_clock::now();
    auto tiempo_openmp = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << " Completado!" << std::endl;
    
    std::cout << std::endl;
    std::cout << "=== VERIFICACIÓN DE RESULTADOS ===" << std::endl;
    
    // Verificar que todos los resultados sean iguales
    bool pthread_correcto = matricesIguales(resultado_secuencial, resultado_pthread);
    bool openmp_correcto = matricesIguales(resultado_secuencial, resultado_openmp);
    
    std::cout << "Resultado pthread correcto: " << (pthread_correcto ? "✓" : "✗") << std::endl;
    std::cout << "Resultado OpenMP correcto:  " << (openmp_correcto ? "✓" : "✗") << std::endl;
    
    if (pthread_correcto && openmp_correcto) {
        std::cout << "✓ Todos los resultados son correctos!" << std::endl;
    } else {
        std::cout << "✗ Error: Los resultados no coinciden!" << std::endl;
    }
    
    // Mostrar matriz resultado pequeña
    if (N <= 10 && P <= 10) {
        imprimirMatriz(resultado_secuencial, "Matriz resultado C");
    }
    
    std::cout << std::endl;
    std::cout << "=== ANÁLISIS DE RENDIMIENTO ===" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Tiempo secuencial: " << tiempo_secuencial.count() << " ms" << std::endl;
    std::cout << "Tiempo pthread:    " << tiempo_pthread.count() << " ms" << std::endl;
    std::cout << "Tiempo OpenMP:     " << tiempo_openmp.count() << " ms" << std::endl;
    
    // Calcular speedup
    double speedup_pthread = static_cast<double>(tiempo_secuencial.count()) / tiempo_pthread.count();
    double speedup_openmp = static_cast<double>(tiempo_secuencial.count()) / tiempo_openmp.count();
    
    std::cout << std::endl;
    std::cout << "Speedup pthread: " << speedup_pthread << "x" << std::endl;
    std::cout << "Speedup OpenMP:  " << speedup_openmp << "x" << std::endl;
    
    // Calcular eficiencia
    double eficiencia_pthread = speedup_pthread / NUM_THREADS;
    double eficiencia_openmp = speedup_openmp / NUM_THREADS;
    
    std::cout << "Eficiencia pthread: " << (eficiencia_pthread * 100) << "%" << std::endl;
    std::cout << "Eficiencia OpenMP:  " << (eficiencia_openmp * 100) << "%" << std::endl;
    
    // Calcular operaciones por segundo
    long long operaciones = 2LL * N * M * P; // Multiplicaciones + sumas
    double ops_sec_secuencial = operaciones / (tiempo_secuencial.count() / 1000.0);
    double ops_sec_pthread = operaciones / (tiempo_pthread.count() / 1000.0);
    double ops_sec_openmp = operaciones / (tiempo_openmp.count() / 1000.0);
    
    std::cout << std::endl;
    std::cout << "Operaciones totales: " << operaciones << std::endl;
    std::cout << "Operaciones/sec secuencial: " << std::fixed << std::setprecision(0) << ops_sec_secuencial << std::endl;
    std::cout << "Operaciones/sec pthread:    " << ops_sec_pthread << std::endl;
    std::cout << "Operaciones/sec OpenMP:     " << ops_sec_openmp << std::endl;
    
    return 0;
}
