#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <pthread.h>
#ifdef NO_OPENMP
    // OpenMP no disponible, usar solo pthread
#else
#include <omp.h>
#endif
#include <iomanip>

// Estructura para pasar datos a los hilos pthread
struct ThreadData {
    const std::vector<int>* array;
    size_t start;
    size_t end;
    long long partial_sum;
};

// Función para generar arreglo grande con números aleatorios
std::vector<int> generarArreglo(size_t size) {
    std::vector<int> array(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000);
    
    for (size_t i = 0; i < size; ++i) {
        array[i] = dis(gen);
    }
    return array;
}

// Función que ejecuta cada hilo pthread
void* sumaParcial(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);
    data->partial_sum = 0;
    
    for (size_t i = data->start; i < data->end; ++i) {
        data->partial_sum += (*data->array)[i];
    }
    
    return nullptr;
}

// Versión secuencial
long long sumaSecuencial(const std::vector<int>& array) {
    long long sum = 0;
    for (int value : array) {
        sum += value;
    }
    return sum;
}

// Versión con pthread
long long sumaPthread(const std::vector<int>& array, int num_threads) {
    std::vector<pthread_t> threads(num_threads);
    std::vector<ThreadData> thread_data(num_threads);
    
    size_t chunk_size = array.size() / num_threads;
    
    // Crear y lanzar hilos
    for (int i = 0; i < num_threads; ++i) {
        thread_data[i].array = &array;
        thread_data[i].start = i * chunk_size;
        thread_data[i].end = (i == num_threads - 1) ? array.size() : (i + 1) * chunk_size;
        
        pthread_create(&threads[i], nullptr, sumaParcial, &thread_data[i]);
    }
    
    // Esperar a que todos los hilos terminen
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
    
    // Sumar resultados parciales
    long long total_sum = 0;
    for (int i = 0; i < num_threads; ++i) {
        total_sum += thread_data[i].partial_sum;
    }
    
    return total_sum;
}

// Versión con OpenMP
long long sumaOpenMP(const std::vector<int>& array) {
    long long sum = 0;
    
    #pragma omp parallel for reduction(+:sum)
    for (size_t i = 0; i < array.size(); ++i) {
        sum += array[i];
    }
    
    return sum;
}

int main() {
    const size_t ARRAY_SIZE = 100000000; // 100 millones de elementos
    const int NUM_THREADS = 8;
    
    std::cout << "=== EJERCICIO 1: SUMA DE ARREGLO GRANDE ===" << std::endl;
    std::cout << "Tamaño del arreglo: " << ARRAY_SIZE << " elementos" << std::endl;
    std::cout << "Número de hilos: " << NUM_THREADS << std::endl;
    std::cout << std::endl;
    
    // Generar arreglo grande
    std::cout << "Generando arreglo de " << ARRAY_SIZE << " elementos...";
    std::cout.flush();
    auto array = generarArreglo(ARRAY_SIZE);
    std::cout << " Completado!" << std::endl;
    
    // Verificar que el arreglo se generó correctamente
    std::cout << "Primeros 5 elementos: ";
    for (int i = 0; i < 5 && i < array.size(); ++i) {
        std::cout << array[i] << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Últimos 5 elementos: ";
    for (int i = std::max(0, static_cast<int>(array.size()) - 5); i < array.size(); ++i) {
        std::cout << array[i] << " ";
    }
    std::cout << std::endl << std::endl;
    
    long long resultado_secuencial, resultado_pthread, resultado_openmp;
    
    // Medir tiempo de versión secuencial
    std::cout << "Ejecutando versión SECUENCIAL...";
    std::cout.flush();
    auto start = std::chrono::high_resolution_clock::now();
    resultado_secuencial = sumaSecuencial(array);
    auto end = std::chrono::high_resolution_clock::now();
    auto tiempo_secuencial = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << " Completado!" << std::endl;
    
    // Medir tiempo de versión pthread
    std::cout << "Ejecutando versión PTHREAD...";
    std::cout.flush();
    start = std::chrono::high_resolution_clock::now();
    resultado_pthread = sumaPthread(array, NUM_THREADS);
    end = std::chrono::high_resolution_clock::now();
    auto tiempo_pthread = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << " Completado!" << std::endl;
    
    // Medir tiempo de versión OpenMP
    std::cout << "Ejecutando versión OPENMP...";
    std::cout.flush();
    start = std::chrono::high_resolution_clock::now();
    resultado_openmp = sumaOpenMP(array);
    end = std::chrono::high_resolution_clock::now();
    auto tiempo_openmp = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << " Completado!" << std::endl;
    
    std::cout << std::endl;
    std::cout << "=== RESULTADOS ===" << std::endl;
    std::cout << "Resultado secuencial: " << resultado_secuencial << std::endl;
    std::cout << "Resultado pthread:    " << resultado_pthread << std::endl;
    std::cout << "Resultado OpenMP:     " << resultado_openmp << std::endl;
    
    // Verificar que todos los resultados sean iguales
    if (resultado_secuencial == resultado_pthread && resultado_secuencial == resultado_openmp) {
        std::cout << "✓ Todos los resultados son correctos!" << std::endl;
    } else {
        std::cout << "✗ Error: Los resultados no coinciden!" << std::endl;
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
    
    return 0;
}
