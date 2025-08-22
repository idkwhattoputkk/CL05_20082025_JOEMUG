# Programación Paralela en C++ - Ejercicios de Evaluación

## Descripción del Proyecto

Este repositorio contiene la implementación completa de tres ejercicios de programación paralela en C++, utilizando tanto **pthread** como **OpenMP** para la paralelización.

## Ejercicios Implementados

### Ejercicio 1: Suma de un Arreglo Grande
- **Descripción**: Cálculo de la suma total de un arreglo de 100 millones de elementos
- **Implementaciones**: Secuencial, pthread con mutex, OpenMP con reduction
- **Archivo**: `ejercicio1_suma_arreglo.cpp`

### Ejercicio 2: Multiplicación de Matrices Paralela
- **Descripción**: Multiplicación de matrices A(n×m) × B(m×p) = C(n×p)
- **Implementaciones**: Secuencial, pthread por filas, OpenMP con collapse
- **Archivo**: `ejercicio2_multiplicacion_matrices.cpp`

### Ejercicio 3: Repositorio de Algoritmos Paralelos Clásicos
- **Descripción**: Cuatro algoritmos fundamentales de programación paralela
- **Algoritmos implementados**:
  1. **Productor-Consumidor**: Buffer compartido con sincronización
  2. **Multiplicación Matriz-Vector**: Paralelización del producto matriz por vector
  3. **Regla Trapezoidal**: Integración numérica paralela usando método trapezoidal
  4. **Count Sort Paralelo**: Algoritmo de ordenamiento por conteo paralelizado
- **Archivo**: `ejercicio3_algoritmos_clasicos.cpp`

## Requisitos del Sistema

### Software Requerido
- **Compilador**: GCC 7.0+ o Clang 5.0+
- **Estándar C++**: C++17 o superior
- **Sistema Operativo**: Linux, macOS, o Windows con WSL

### Librerías
- **pthread**: Incluida en la biblioteca estándar de C
- **OpenMP**: Incluida en GCC/Clang modernos

### Hardware Recomendado
- **Procesador**: Mínimo 4 núcleos para mejor rendimiento
- **Memoria**: Mínimo 8GB RAM (especialmente para matrices grandes)

## Instalación y Compilación

### 1. Clonar el Repositorio
```bash
git clone https://github.com/idkwhattoputkk/CL05_20082025_JOEMUG.git
cd CL05_20082025_JOEMUG
```

### 2. Compilar Todos los Ejercicios
```bash
make all
```

### 3. Compilar Ejercicios Individuales
```bash
make ejercicio1_suma_arreglo
make ejercicio2_multiplicacion_matrices
make ejercicio3_algoritmos_clasicos
```

## Ejecución de los Programas

### Ejecutar Todos los Ejercicios
```bash
make run-all
```

### Ejecutar Ejercicios Individuales
```bash
make run-1    # Ejercicio 1: Suma de arreglo grande
make run-2    # Ejercicio 2: Multiplicación de matrices
make run-3    # Ejercicio 3: Algoritmos clásicos
```

### Ejecutar con Diferentes Números de Hilos
```bash
make test-threads
OMP_NUM_THREADS=4 ./ejercicio1_suma_arreglo
```

## Análisis de Rendimiento

### Métricas Calculadas
- **Tiempo de ejecución**: Medido en milisegundos usando `chrono`
- **Speedup**: Relación entre tiempo secuencial y paralelo
- **Eficiencia**: Speedup dividido por el número de hilos
- **Operaciones por segundo**: Para algoritmos computacionales

### Interpretación de Resultados
- **Speedup > 1**: Mejora en rendimiento
- **Speedup = N**: Speedup lineal ideal (donde N = número de hilos)
- **Eficiencia > 80%**: Buena utilización de recursos paralelos
- **Eficiencia < 50%**: Posible overhead de paralelización

## Opciones de Compilación

### Compilación con Debug
```bash
make debug
```

### Compilación con Optimizaciones Máximas
```bash
make release
```

### Compilación Personalizada
```bash
g++ -std=c++17 -O3 -pthread -fopenmp -o ejercicio1 ejercicio1_suma_arreglo.cpp
```

## Estructura de Archivos

```
CL05_20082025_JOEMUG/
├── ejercicio1_suma_arreglo.cpp      # Suma de arreglo grande
├── ejercicio2_multiplicacion_matrices.cpp  # Multiplicación de matrices
├── ejercicio3_algoritmos_clasicos.cpp      # Algoritmos clásicos
├── Makefile                          # Sistema de compilación
├── README.md                         # Este archivo
└── RESUMEN_RESULTADOS.md             # Análisis de rendimiento
```

## Casos de Prueba

### Tamaños de Datos de Prueba
- **Ejercicio 1**: 100,000,000 elementos (100M)
- **Ejercicio 2**: Matrices 1000×1000
- **Ejercicio 3**: 
  - Productor-Consumidor: 3 productores, 2 consumidores, buffer de 10
  - Matriz-Vector: 2000×2000
  - Regla Trapezoidal: 10,000,000 trapecios
  - Count Sort: 1,000,000 elementos

### Configuraciones de Hilos
- **Por defecto**: 8 hilos
- **Configurable**: Variable `NUM_THREADS` en el código
- **OpenMP**: Variable de entorno `OMP_NUM_THREADS`

## Solución de Problemas

### Errores Comunes

#### Error de Compilación: pthread no encontrado
```bash
# En Ubuntu/Debian
sudo apt-get install build-essential

# En CentOS/RHEL
sudo yum groupinstall "Development Tools"
```

#### Error de Compilación: OpenMP no encontrado
```bash
# En Ubuntu/Debian
sudo apt-get install libomp-dev

# En CentOS/RHEL
sudo yum install libomp-devel
```

#### Error de Ejecución: Memoria insuficiente
```bash
# Reducir tamaño de datos en el código
const size_t ARRAY_SIZE = 10000000; // 10M en lugar de 100M
```

## Conceptos Teóricos Implementados

### Estrategias de Paralelización
1. **División de datos**: Segmentación de arreglos y matrices
2. **Sincronización**: Mutex, variables de condición, barreras
3. **Balance de carga**: Distribución equitativa de trabajo
4. **Reducción**: Acumulación segura de resultados parciales

### Patrones de Diseño
1. **Fork-Join**: Crear hilos, esperar terminación
2. **Producer-Consumer**: Buffer compartido con sincronización
3. **Data Parallel**: Mismo código en diferentes datos
4. **Task Parallel**: Diferentes tareas en paralelo

## Referencias y Recursos

### Documentación
- [OpenMP Specification](https://www.openmp.org/specifications/)
- [POSIX Threads Programming](https://computing.llnl.gov/tutorials/pthreads/)
- [C++ Standard Library](https://en.cppreference.com/)

### Herramientas de Análisis
- **gprof**: Profiling de rendimiento
- **valgrind**: Análisis de memoria
- **perf**: Análisis de rendimiento del kernel

## Licencia

Este proyecto está bajo licencia educativa para uso académico.

## Autor

Desarrollado para el curso de Programación Paralela.

---

**Nota**: Este código está diseñado para fines educativos y de demostración. Para uso en producción, se recomienda optimización adicional y manejo de errores más robusto.
