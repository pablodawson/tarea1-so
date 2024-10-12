// Tarea 1: Creación y comunicación de procesos.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <time.h>
#include <string.h>

#include <sys/mman.h> // Para funciones de memoria compartida
#include <fcntl.h>    // Para constantes de control de archivos

// Función para Bubble Sort
void bubble_sort(int arr[], int n) {
    int i, j, temp;
    for (i = 0; i < n-1; i++){
        for (j = 0; j < n-i-1; j++){
            if (arr[j] > arr[j+1]) {
                // Intercambiar arr[j] y arr[j+1]
                temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}

// Funciones para Quick Sort
void swap(int* a, int* b) {
    int t = *a; *a = *b; *b = t;
}

int partition (int arr[], int low, int high)
{
    int pivot = arr[high]; // Pivote
    int i = (low - 1); // Índice de elemento más pequeño

    for (int j = low; j <= high - 1; j++)
    {
        // Si el elemento actual es menor o igual al pivote
        if (arr[j] <= pivot)
        {
            i++;    // Incrementar índice de elemento más pequeño
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quick_sort(int arr[], int low, int high){
    if (low < high)
    {
        int pi = partition(arr, low, high);

        // Ordenar recursivamente las particiones
        quick_sort(arr, low, pi - 1);
        quick_sort(arr, pi + 1, high);
    }
}

int main(int argc, char* argv[]){

    // Inicializar generador de números aleatorios
    srand(time(NULL));

    // Definir número de hijos y tamaño de datos
    const int NUM_HIJOS = 4;
    const int TAMANO_DATOS = 50;

    // Nombre del objeto de memoria compartida
    const char *nombre_memoria = "MemoriaCompartida";

    // Tamaño de la memoria compartida
    size_t tamaño_memoria = NUM_HIJOS * TAMANO_DATOS * sizeof(int);

    // Crear objeto de memoria compartida
    int shm_fd = shm_open(nombre_memoria, O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1){
        perror("Error al crear memoria compartida");
        exit(1);
    }

    // Configurar el tamaño de la memoria compartida
    if(ftruncate(shm_fd, tamaño_memoria) == -1){
        perror("Error al configurar tamaño de memoria compartida");
        exit(1);
    }

    // Mapear la memoria compartida
    int *datos_compartidos = mmap(NULL, tamaño_memoria, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(datos_compartidos == MAP_FAILED){
        perror("Error al mapear memoria compartida");
        exit(1);
    }

    // Generar números aleatorios y almacenarlos en memoria compartida
    for(int i = 0; i < NUM_HIJOS * TAMANO_DATOS; i++){
        datos_compartidos[i] = rand() % 1000 + 1; // Números entre 1 y 1000
    }

    // Crear procesos hijos
    pid_t pids[NUM_HIJOS];

    for(int i = 0; i < NUM_HIJOS; i++){
        pids[i] = fork();

        if(pids[i] < 0){
            perror("Error al crear proceso hijo");
            exit(1);
        }
        else if(pids[i] == 0){
            // Proceso hijo

            // Calcular el índice inicial en memoria compartida para este hijo
            int *datos_hijo = datos_compartidos + i * TAMANO_DATOS;

            // Esperar un tiempo aleatorio entre 10 y 30 segundos
            srand(time(NULL) + i); // Inicializar la semilla del generador de números aleatorios con un valor diferente para cada hijo
            int tiempo_espera = rand() % 21 + 10; // Entre 10 y 30 segundos
            printf("Hijo %d va a esperar %d segundos\n", i+1, tiempo_espera);
            sleep(tiempo_espera);

            // Realizar ordenamiento
            if(i < 2){
                // Primeros dos hijos usan QuickSort
                printf("Hijo %d va a usar QuickSort\n", i+1);
                quick_sort(datos_hijo, 0, TAMANO_DATOS -1);
            } else {
                // Últimos dos hijos usan Bubble Sort
                printf("Hijo %d va a usar Bubble Sort\n", i+1);
                bubble_sort(datos_hijo, TAMANO_DATOS);
            }

            // Desmapear memoria compartida
            munmap(datos_compartidos, tamaño_memoria);

            // Salir del proceso hijo
            exit(0);
        }
        // El proceso padre continúa al siguiente hijo
    }

    // El proceso padre espera a que todos los hijos terminen
    for(int i = 0; i < NUM_HIJOS; i++){
        waitpid(pids[i], NULL, 0);

        // Una vez que el hijo ha terminado, leer y mostrar los datos ordenados
        int *datos_hijo = datos_compartidos + i * TAMANO_DATOS;

        printf("Datos ordenados del hijo %d:\n", i+1);
        for(int j = 0; j < TAMANO_DATOS; j++){
            printf("%d ", datos_hijo[j]);
        }
        printf("\n");
    }

    // Limpiar memoria compartida
    munmap(datos_compartidos, tamaño_memoria);
    shm_unlink(nombre_memoria);

    return 0;
}

