# Makefile para Programación Paralela - C++
# Compilador y opciones
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
PTHREAD_FLAGS = -pthread
OPENMP_FLAGS = -fopenmp

# Detectar si OpenMP está disponible
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # macOS - usar clang con OpenMP
    OPENMP_FLAGS = -Xpreprocessor -fopenmp -lomp
    OPENMP_LDFLAGS = -L/usr/local/opt/libomp/lib
    OPENMP_CPPFLAGS = -I/usr/local/opt/libomp/include
    # Verificar si OpenMP está disponible
    OPENMP_AVAILABLE := $(shell g++ $(OPENMP_CPPFLAGS) $(OPENMP_LDFLAGS) -Xpreprocessor -fopenmp -lomp -E -x c++ /dev/null >/dev/null 2>&1 && echo "yes" || echo "no")
else
    # Linux - usar gcc con OpenMP
    OPENMP_AVAILABLE := $(shell g++ -fopenmp -E -x c++ /dev/null >/dev/null 2>&1 && echo "yes" || echo "no")
    OPENMP_LDFLAGS = 
    OPENMP_CPPFLAGS = 
endif

# Si OpenMP no está disponible, usar solo pthread
ifeq ($(OPENMP_AVAILABLE),no)
    OPENMP_FLAGS = 
    $(warning OpenMP no disponible, compilando solo con pthread)
endif

# Nombres de los ejecutables
EJERCICIO1 = ejercicio1_suma_arreglo
EJERCICIO2 = ejercicio2_multiplicacion_matrices
EJERCICIO3 = ejercicio3_algoritmos_clasicos

# Todos los ejecutables
ALL = $(EJERCICIO1) $(EJERCICIO2) $(EJERCICIO3)

# Regla por defecto
all: $(ALL)

# Ejercicio 1: Suma de arreglo grande
$(EJERCICIO1): ejercicio1_suma_arreglo.cpp
	$(CXX) $(CXXFLAGS) $(OPENMP_CPPFLAGS) $(PTHREAD_FLAGS) $(OPENMP_FLAGS) $(OPENMP_LDFLAGS) -o $@ $<

# Ejercicio 2: Multiplicación de matrices
$(EJERCICIO2): ejercicio2_multiplicacion_matrices.cpp
	$(CXX) $(CXXFLAGS) $(OPENMP_CPPFLAGS) $(PTHREAD_FLAGS) $(OPENMP_FLAGS) $(OPENMP_LDFLAGS) -o $@ $<

# Ejercicio 3: Algoritmos clásicos
$(EJERCICIO3): ejercicio3_algoritmos_clasicos.cpp
	$(CXX) $(CXXFLAGS) $(OPENMP_CPPFLAGS) $(PTHREAD_FLAGS) $(OPENMP_FLAGS) $(OPENMP_LDFLAGS) -o $@ $<

# Regla para compilar solo con pthread (sin OpenMP)
pthread-only: CXXFLAGS += -DNO_OPENMP
pthread-only: $(ALL)

# Regla para limpiar archivos generados
clean:
	rm -f $(ALL)

# Regla para ejecutar todos los ejercicios
run-all: $(ALL)
	@echo "=== EJECUTANDO TODOS LOS EJERCICIOS ==="
	@echo ""
	@echo "Ejercicio 1: Suma de arreglo grande"
	@echo "====================================="
	./$(EJERCICIO1)
	@echo ""
	@echo "Ejercicio 2: Multiplicación de matrices"
	@echo "======================================="
	./$(EJERCICIO2)
	@echo ""
	@echo "Ejercicio 3: Algoritmos clásicos"
	@echo "================================="
	./$(EJERCICIO3)

# Regla para ejecutar ejercicios individuales
run-1: $(EJERCICIO1)
	./$(EJERCICIO1)

run-2: $(EJERCICIO2)
	./$(EJERCICIO2)

run-3: $(EJERCICIO3)
	./$(EJERCICIO3)

# Regla para mostrar información del sistema
info:
	@echo "=== INFORMACIÓN DEL SISTEMA ==="
	@echo "Compilador: $(CXX)"
	@echo "Versión del compilador:"
	@$(CXX) --version | head -1
	@echo ""
	@echo "Número de núcleos disponibles:"
	@nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo "No se pudo determinar"
	@echo ""
	@echo "Memoria disponible:"
	@free -h 2>/dev/null || vm_stat 2>/dev/null || echo "No se pudo determinar"

# Regla para verificar dependencias
check-deps:
	@echo "=== VERIFICANDO DEPENDENCIAS ==="
	@echo "Verificando pthread..."
	@$(CXX) $(CXXFLAGS) $(PTHREAD_FLAGS) -E -x c++ /dev/null >/dev/null 2>&1 && echo "✓ pthread disponible" || echo "✗ pthread no disponible"
	@echo "Verificando OpenMP..."
	@$(CXX) $(CXXFLAGS) $(OPENMP_FLAGS) -E -x c++ /dev/null >/dev/null 2>&1 && echo "✓ OpenMP disponible" || echo "✗ OpenMP no disponible"

# Regla para mostrar ayuda
help:
	@echo "=== AYUDA DEL MAKEFILE ==="
	@echo "Comandos disponibles:"
	@echo "  make all          - Compilar todos los ejercicios"
	@echo "  make clean        - Limpiar archivos compilados"
	@echo "  make run-all      - Compilar y ejecutar todos los ejercicios"
	@echo "  make run-1        - Compilar y ejecutar solo el ejercicio 1"
	@echo "  make run-2        - Compilar y ejecutar solo el ejercicio 2"
	@echo "  make run-3        - Compilar y ejecutar solo el ejercicio 3"
	@echo "  make info         - Mostrar información del sistema"
	@echo "  make check-deps   - Verificar dependencias disponibles"
	@echo "  make help         - Mostrar esta ayuda"

# Regla para compilar con optimizaciones de debug
debug: CXXFLAGS += -g -DDEBUG
debug: $(ALL)

# Regla para compilar con optimizaciones máximas
release: CXXFLAGS += -O3 -march=native
release: $(ALL)

# Regla para compilar con diferentes números de hilos
test-threads: $(EJERCICIO1)
	@echo "=== PROBANDO DIFERENTES NÚMEROS DE HILOS ==="
	@for threads in 1 2 4 8 16; do \
		echo "Probando con $$threads hilos:"; \
		OMP_NUM_THREADS=$$threads ./$(EJERCICIO1) | grep -E "(Speedup|Eficiencia)" || true; \
		echo ""; \
	done

.PHONY: all clean run-all run-1 run-2 run-3 info check-deps help debug release test-threads
