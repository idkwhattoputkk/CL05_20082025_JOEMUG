#include <iostream>
#include <vector>
#include <queue>
#include <random>
#include <chrono>
#include <pthread.h>
#include <omp.h>
#include <iomanip>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <thread>

// ============================================================================
// 1. PROBLEMA PRODUCTOR-CONSUMIDOR
// ============================================================================

class BufferProductorConsumidor {
private:
    std::queue<int> buffer;
    std::mutex mutex;
    std::condition_variable not_full;
    std::condition_variable not_empty;
    size_t max_size;
    bool done;

public:
    BufferProductorConsumidor(size_t size) : max_size(size), done(false) {}
    
    void producir(int item) {
        std::unique_lock<std::mutex> lock(mutex);
        not_full.wait(lock, [this] { return buffer.size() < max_size || done; });
        
        if (!done) {
            buffer.push(item);
            not_empty.notify_one();
        }
    }
    
    bool consumir(int& item) {
        std::unique_lock<std::mutex> lock(mutex);
        not_empty.wait(lock, [this] { return !buffer.empty() || done; });
        
        if (buffer.empty() && done) {
            return false;
        }
        
        item = buffer.front();
        buffer.pop();
        not_full.notify_one();
        return true;
    }
    
    void terminar() {
        std::unique_lock<std::mutex> lock(mutex);
        done = true;
        not_full.notify_all();
        not_empty.notify_all();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex));
        return buffer.size();
    }
};

struct ProductorConsumidorData {
    BufferProductorConsumidor* buffer;
    int id;
    int num_items;
};

void* productor(void* arg) {
    ProductorConsumidorData* data = static_cast<ProductorConsumidorData*>(arg);
    
    for (int i = 0; i < data->num_items; ++i) {
        int item = data->id * 1000 + i;
        data->buffer->producir(item);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    return nullptr;
}

void* consumidor(void* arg) {
    ProductorConsumidorData* data = static_cast<ProductorConsumidorData*>(arg);
    
    int item;
    while (data->buffer->consumir(item)) {
        // Procesar item
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    
    return nullptr;
}

void ejecutarProductorConsumidor() {
    std::cout << "\n=== 1. PROBLEMA PRODUCTOR-CONSUMIDOR ===" << std::endl;
    
    const int BUFFER_SIZE = 10;
    const int NUM_PRODUCTORES = 3;
    const int NUM_CONSUMIDORES = 2;
    const int ITEMS_POR_PRODUCTOR = 20;
    
    BufferProductorConsumidor buffer(BUFFER_SIZE);
    std::vector<pthread_t> productores(NUM_PRODUCTORES);
    std::vector<pthread_t> consumidores(NUM_CONSUMIDORES);
    std::vector<ProductorConsumidorData> prod_data(NUM_PRODUCTORES);
    std::vector<ProductorConsumidorData> cons_data(NUM_CONSUMIDORES);
    
    // Crear productores
    for (int i = 0; i < NUM_PRODUCTORES; ++i) {
        prod_data[i] = {&buffer, i, ITEMS_POR_PRODUCTOR};
        pthread_create(&productores[i], nullptr, productor, &prod_data[i]);
    }
    
    // Crear consumidores
    for (int i = 0; i < NUM_CONSUMIDORES; ++i) {
        cons_data[i] = {&buffer, i, 0};
        pthread_create(&consumidores[i], nullptr, consumidor, &cons_data[i]);
    }
    
    // Esperar a que los productores terminen
    for (int i = 0; i < NUM_PRODUCTORES; ++i) {
        pthread_join(productores[i], nullptr);
    }
    
    // Señalar que no hay más producción
    buffer.terminar();
    
    // Esperar a que los consumidores terminen
    for (int i = 0; i < NUM_CONSUMIDORES; ++i) {
        pthread_join(consumidores[i], nullptr);
    }
    
    std::cout << "Productor-Consumidor completado exitosamente!" << std::endl;
}

// ============================================================================
// 2. MULTIPLICACIÓN MATRIZ-VECTOR
// ============================================================================

struct MatrizVectorData {
    const std::vector<std::vector<int>>* matriz;
    const std::vector<int>* vector;
    std::vector<int>* resultado;
    int start_row;
    int end_row;
};

void* multiplicarMatrizVectorParcial(void* arg) {
    MatrizVectorData* data = static_cast<MatrizVectorData*>(arg);
    
    for (int i = data->start_row; i < data->end_row; ++i) {
        (*data->resultado)[i] = 0;
        for (size_t j = 0; j < (*data->matriz)[i].size(); ++j) {
            (*data->resultado)[i] += (*data->matriz)[i][j] * (*data->vector)[j];
        }
    }
    
    return nullptr;
}

std::vector<int> multiplicarMatrizVectorParalelo(
    const std::vector<std::vector<int>>& matriz,
    const std::vector<int>& vector,
    int num_threads) {
    
    int n = matriz.size();
    std::vector<int> resultado(n, 0);
    std::vector<pthread_t> threads(num_threads);
    std::vector<MatrizVectorData> thread_data(num_threads);
    
    int chunk_size = n / num_threads;
    
    // Crear y lanzar hilos
    for (int i = 0; i < num_threads; ++i) {
        thread_data[i].matriz = &matriz;
        thread_data[i].vector = &vector;
        thread_data[i].resultado = &resultado;
        thread_data[i].start_row = i * chunk_size;
        thread_data[i].end_row = (i == num_threads - 1) ? n : (i + 1) * chunk_size;
        
        pthread_create(&threads[i], nullptr, multiplicarMatrizVectorParcial, &thread_data[i]);
    }
    
    // Esperar a que todos los hilos terminen
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
    
    return resultado;
}

std::vector<int> multiplicarMatrizVectorSecuencial(
    const std::vector<std::vector<int>>& matriz,
    const std::vector<int>& vector) {
    
    int n = matriz.size();
    std::vector<int> resultado(n, 0);
    
    for (int i = 0; i < n; ++i) {
        for (size_t j = 0; j < matriz[i].size(); ++j) {
            resultado[i] += matriz[i][j] * vector[j];
        }
    }
    
    return resultado;
}

void ejecutarMultiplicacionMatrizVector() {
    std::cout << "\n=== 2. MULTIPLICACIÓN MATRIZ-VECTOR ===" << std::endl;
    
    const int N = 2000; // Filas de matriz
    const int M = 2000; // Columnas de matriz
    const int NUM_THREADS = 8;
    
    // Generar matriz y vector
    std::vector<std::vector<int>> matriz(N, std::vector<int>(M));
    std::vector<int> vector(M);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < M; ++j) {
            matriz[i][j] = dis(gen);
        }
    }
    
    for (int j = 0; j < M; ++j) {
        vector[j] = dis(gen);
    }
    
    std::cout << "Matriz: " << N << " x " << M << std::endl;
    std::cout << "Vector: " << M << " elementos" << std::endl;
    
    // Medir versión secuencial
    auto start = std::chrono::high_resolution_clock::now();
    auto resultado_secuencial = multiplicarMatrizVectorSecuencial(matriz, vector);
    auto end = std::chrono::high_resolution_clock::now();
    auto tiempo_secuencial = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Medir versión paralela
    start = std::chrono::high_resolution_clock::now();
    auto resultado_paralelo = multiplicarMatrizVectorParalelo(matriz, vector, NUM_THREADS);
    end = std::chrono::high_resolution_clock::now();
    auto tiempo_paralelo = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Verificar resultados
    bool correcto = (resultado_secuencial == resultado_paralelo);
    std::cout << "Resultado correcto: " << (correcto ? "✓" : "✗") << std::endl;
    
    // Análisis de rendimiento
    double speedup = static_cast<double>(tiempo_secuencial.count()) / tiempo_paralelo.count();
    double eficiencia = speedup / NUM_THREADS;
    
    std::cout << "Tiempo secuencial: " << tiempo_secuencial.count() << " ms" << std::endl;
    std::cout << "Tiempo paralelo:   " << tiempo_paralelo.count() << " ms" << std::endl;
    std::cout << "Speedup: " << speedup << "x" << std::endl;
    std::cout << "Eficiencia: " << (eficiencia * 100) << "%" << std::endl;
}

// ============================================================================
// 3. REGLA TRAPEZOIDAL (INTEGRACIÓN NUMÉRICA)
// ============================================================================

double funcion(double x) {
    return x * x; // f(x) = x²
}

double reglaTrapezoidalSecuencial(double a, double b, int n) {
    double h = (b - a) / n;
    double suma = 0.5 * (funcion(a) + funcion(b));
    
    for (int i = 1; i < n; ++i) {
        double x = a + i * h;
        suma += funcion(x);
    }
    
    return h * suma;
}

struct TrapezoidalData {
    double a, b;
    int n;
    int start_i, end_i;
    double partial_sum;
};

void* reglaTrapezoidalParcial(void* arg) {
    TrapezoidalData* data = static_cast<TrapezoidalData*>(arg);
    
    double h = (data->b - data->a) / data->n;
    data->partial_sum = 0.0;
    
    for (int i = data->start_i; i < data->end_i; ++i) {
        double x = data->a + i * h;
        data->partial_sum += funcion(x);
    }
    
    return nullptr;
}

double reglaTrapezoidalParalela(double a, double b, int n, int num_threads) {
    std::vector<pthread_t> threads(num_threads);
    std::vector<TrapezoidalData> thread_data(num_threads);
    
    int chunk_size = n / num_threads;
    
    // Crear y lanzar hilos
    for (int i = 0; i < num_threads; ++i) {
        thread_data[i].a = a;
        thread_data[i].b = b;
        thread_data[i].n = n;
        thread_data[i].start_i = i * chunk_size;
        thread_data[i].end_i = (i == num_threads - 1) ? n : (i + 1) * chunk_size;
        
        pthread_create(&threads[i], nullptr, reglaTrapezoidalParcial, &thread_data[i]);
    }
    
    // Esperar a que todos los hilos terminen
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
    
    // Calcular resultado final
    double h = (b - a) / n;
    double suma = 0.5 * (funcion(a) + funcion(b));
    
    for (int i = 0; i < num_threads; ++i) {
        suma += thread_data[i].partial_sum;
    }
    
    return h * suma;
}

void ejecutarReglaTrapezoidal() {
    std::cout << "\n=== 3. REGLA TRAPEZOIDAL (INTEGRACIÓN NUMÉRICA) ===" << std::endl;
    
    const double A = 0.0; // Límite inferior
    const double B = 1.0; // Límite superior
    const int N = 10000000; // Número de trapecios
    const int NUM_THREADS = 8;
    
    std::cout << "Integrando f(x) = x² desde " << A << " hasta " << B << std::endl;
    std::cout << "Número de trapecios: " << N << std::endl;
    std::cout << "Valor teórico: " << (B*B*B - A*A*A) / 3.0 << std::endl;
    
    // Medir versión secuencial
    auto start = std::chrono::high_resolution_clock::now();
    double resultado_secuencial = reglaTrapezoidalSecuencial(A, B, N);
    auto end = std::chrono::high_resolution_clock::now();
    auto tiempo_secuencial = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Medir versión paralela
    start = std::chrono::high_resolution_clock::now();
    double resultado_paralelo = reglaTrapezoidalParalela(A, B, N, NUM_THREADS);
    end = std::chrono::high_resolution_clock::now();
    auto tiempo_paralelo = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Resultado secuencial: " << std::fixed << std::setprecision(10) << resultado_secuencial << std::endl;
    std::cout << "Resultado paralelo:   " << resultado_paralelo << std::endl;
    
    // Análisis de rendimiento
    double speedup = static_cast<double>(tiempo_secuencial.count()) / tiempo_paralelo.count();
    double eficiencia = speedup / NUM_THREADS;
    
    std::cout << "Tiempo secuencial: " << tiempo_secuencial.count() << " ms" << std::endl;
    std::cout << "Tiempo paralelo:   " << tiempo_paralelo.count() << " ms" << std::endl;
    std::cout << "Speedup: " << speedup << "x" << std::endl;
    std::cout << "Eficiencia: " << (eficiencia * 100) << "%" << std::endl;
}

// ============================================================================
// 4. COUNT SORT PARALELO
// ============================================================================

struct CountSortData {
    const std::vector<int>* input;
    std::vector<int>* output;
    std::vector<int>* count;
    int start_val, end_val;
    int min_val;
};

void* countSortParcial(void* arg) {
    CountSortData* data = static_cast<CountSortData*>(arg);
    
    // Contar elementos en el rango asignado
    for (int val : *data->input) {
        if (val >= data->start_val && val < data->end_val) {
            (*data->count)[val - data->min_val]++;
        }
    }
    
    return nullptr;
}

std::vector<int> countSortParalelo(const std::vector<int>& input, int num_threads) {
    if (input.empty()) return {};
    
    int min_val = *std::min_element(input.begin(), input.end());
    int max_val = *std::max_element(input.begin(), input.end());
    int range = max_val - min_val + 1;
    
    std::vector<int> output(input.size());
    std::vector<int> count(range, 0);
    
    std::vector<pthread_t> threads(num_threads);
    std::vector<CountSortData> thread_data(num_threads);
    
    int chunk_size = range / num_threads;
    
    // Crear y lanzar hilos para contar
    for (int i = 0; i < num_threads; ++i) {
        thread_data[i].input = &input;
        thread_data[i].output = &output;
        thread_data[i].count = &count;
        thread_data[i].start_val = min_val + i * chunk_size;
        thread_data[i].end_val = (i == num_threads - 1) ? max_val + 1 : min_val + (i + 1) * chunk_size;
        thread_data[i].min_val = min_val;
        
        pthread_create(&threads[i], nullptr, countSortParcial, &thread_data[i]);
    }
    
    // Esperar a que todos los hilos terminen
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
    
    // Calcular posiciones acumulativas
    for (int i = 1; i < range; ++i) {
        count[i] += count[i - 1];
    }
    
    // Colocar elementos en sus posiciones correctas
    for (int i = input.size() - 1; i >= 0; --i) {
        int val = input[i];
        int pos = --count[val - min_val];
        output[pos] = val;
    }
    
    return output;
}

std::vector<int> countSortSecuencial(const std::vector<int>& input) {
    if (input.empty()) return {};
    
    int min_val = *std::min_element(input.begin(), input.end());
    int max_val = *std::max_element(input.begin(), input.end());
    int range = max_val - min_val + 1;
    
    std::vector<int> count(range, 0);
    std::vector<int> output(input.size());
    
    // Contar elementos
    for (int val : input) {
        count[val - min_val]++;
    }
    
    // Calcular posiciones acumulativas
    for (int i = 1; i < range; ++i) {
        count[i] += count[i - 1];
    }
    
    // Colocar elementos en sus posiciones correctas
    for (int i = input.size() - 1; i >= 0; --i) {
        int val = input[i];
        int pos = --count[val - min_val];
        output[pos] = val;
    }
    
    return output;
}

void ejecutarCountSort() {
    std::cout << "\n=== 4. COUNT SORT PARALELO ===" << std::endl;
    
    const int ARRAY_SIZE = 1000000; // 1 millón de elementos
    const int NUM_THREADS = 8;
    
    // Generar arreglo de prueba
    std::vector<int> input(ARRAY_SIZE);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 10000);
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        input[i] = dis(gen);
    }
    
    std::cout << "Tamaño del arreglo: " << ARRAY_SIZE << " elementos" << std::endl;
    std::cout << "Rango de valores: 1 - 10000" << std::endl;
    
    // Verificar que el arreglo no esté ordenado
    bool ordenado = std::is_sorted(input.begin(), input.end());
    std::cout << "Arreglo original ordenado: " << (ordenado ? "Sí" : "No") << std::endl;
    
    // Medir versión secuencial
    auto start = std::chrono::high_resolution_clock::now();
    auto resultado_secuencial = countSortSecuencial(input);
    auto end = std::chrono::high_resolution_clock::now();
    auto tiempo_secuencial = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Medir versión paralela
    start = std::chrono::high_resolution_clock::now();
    auto resultado_paralelo = countSortParalelo(input, NUM_THREADS);
    end = std::chrono::high_resolution_clock::now();
    auto tiempo_paralelo = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Verificar resultados
    bool correcto = (resultado_secuencial == resultado_paralelo);
    bool ordenado_secuencial = std::is_sorted(resultado_secuencial.begin(), resultado_secuencial.end());
    bool ordenado_paralelo = std::is_sorted(resultado_paralelo.begin(), resultado_paralelo.end());
    
    std::cout << "Resultado correcto: " << (correcto ? "✓" : "✗") << std::endl;
    std::cout << "Secuencial ordenado: " << (ordenado_secuencial ? "✓" : "✗") << std::endl;
    std::cout << "Paralelo ordenado:   " << (ordenado_paralelo ? "✓" : "✗") << std::endl;
    
    // Análisis de rendimiento
    double speedup = static_cast<double>(tiempo_secuencial.count()) / tiempo_paralelo.count();
    double eficiencia = speedup / NUM_THREADS;
    
    std::cout << "Tiempo secuencial: " << tiempo_secuencial.count() << " ms" << std::endl;
    std::cout << "Tiempo paralelo:   " << tiempo_paralelo.count() << " ms" << std::endl;
    std::cout << "Speedup: " << speedup << "x" << std::endl;
    std::cout << "Eficiencia: " << (eficiencia * 100) << "%" << std::endl;
}

// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================

int main() {
    std::cout << "=== REPOSITORIO DE ALGORITMOS PARALELOS CLÁSICOS ===" << std::endl;
    std::cout << "Implementando 4 algoritmos fundamentales de programación paralela" << std::endl;
    
    try {
        // Ejecutar todos los algoritmos
        ejecutarProductorConsumidor();
        ejecutarMultiplicacionMatrizVector();
        ejecutarReglaTrapezoidal();
        ejecutarCountSort();
        
        std::cout << "\n=== TODOS LOS ALGORITMOS COMPLETADOS EXITOSAMENTE ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error durante la ejecución: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
