#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

// Valor máximo de clave
const int CLAVE_MAXIMA = 65536;
const int MAXIMO_HILOS = 1024;

struct nodo {
   int dato;
   struct nodo* siguiente;
};

// Variables compartidas
struct nodo* cabeza = NULL;
int n = 1000;   // Número inicial de elementos en la lista
int m = 10000;  // Total de operaciones aleatorias

// Fracciones de operaciones
float fracMiembro = 0.50;   // Fracción para la operación Miembro
float fracInsertar = 0.25;  // Fracción para la operación Insertar
float fracEliminar = 0.25;  // Fracción para la operación Eliminar

int cantidad_hilos = 1;   // Cantidad de hilos

double tiempo_inicio, tiempo_fin, tiempo_total;

int conteo_miembro = 0; // Conteo de llamadas a función Miembro
int conteo_insertar = 0; // Conteo de llamadas a función Insertar
int conteo_eliminar = 0; // Conteo de llamadas a función Eliminar

pthread_mutex_t mutex_conteo;

// Estructura de bloqueo de lectura-escritura personalizada
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t lectores;
    pthread_cond_t escritores;
    int cantidad_lectores;
    int escritores_activos;
} rw_lock_t;

rw_lock_t bloqueo_personalizado;

// Prototipos de funciones
int Miembro(int valor);
int Insertar(int valor);
int Eliminar(int valor);
void Limpiar_Memoria(void);
int Esta_Vacia(void);
void* Funcion_Hilo(void* rango);

// Funciones para inicializar, bloquear y desbloquear el rw_lock personalizado
void rwlock_init(rw_lock_t* rw) {
    pthread_mutex_init(&rw->mutex, NULL);
    pthread_cond_init(&rw->lectores, NULL);
    pthread_cond_init(&rw->escritores, NULL);
    rw->cantidad_lectores = 0;
    rw->escritores_activos = 0;
}

void rwlock_lectura_bloquear(rw_lock_t* rw) {
    pthread_mutex_lock(&rw->mutex);
    while (rw->escritores_activos > 0) {
        pthread_cond_wait(&rw->lectores, &rw->mutex);
    }
    rw->cantidad_lectores++;
    pthread_mutex_unlock(&rw->mutex);
}

void rwlock_lectura_desbloquear(rw_lock_t* rw) {
    pthread_mutex_lock(&rw->mutex);
    rw->cantidad_lectores--;
    if (rw->cantidad_lectores == 0) {
        pthread_cond_signal(&rw->escritores);
    }
    pthread_mutex_unlock(&rw->mutex);
}

void rwlock_escritura_bloquear(rw_lock_t* rw) {
    pthread_mutex_lock(&rw->mutex);
    while (rw->cantidad_lectores > 0 || rw->escritores_activos > 0) {
        pthread_cond_wait(&rw->escritores, &rw->mutex);
    }
    rw->escritores_activos = 1;
    pthread_mutex_unlock(&rw->mutex);
}

void rwlock_escritura_desbloquear(rw_lock_t* rw) {
    pthread_mutex_lock(&rw->mutex);
    rw->escritores_activos = 0;
    pthread_cond_signal(&rw->escritores);
    pthread_cond_broadcast(&rw->lectores);
    pthread_mutex_unlock(&rw->mutex);
}

int main(int argc, char* argv[]) {
    if (argc != 2) { 
        fprintf(stderr, "Proporcione un argumento para la cantidad de hilos (menos de %d)\n", MAXIMO_HILOS);
        exit(0);
    }

    cantidad_hilos = strtol(argv[1], NULL, 10);  
    if (cantidad_hilos <= 0 || cantidad_hilos > MAXIMO_HILOS) { 
        fprintf(stderr, "Proporcione un argumento para la cantidad de hilos (menos de %d)\n", MAXIMO_HILOS);
        exit(0);
    }

    int i = 0;
    // Insertar elementos en la lista enlazada
    for (; i < n; i++) {   
        int r = rand() % CLAVE_MAXIMA;
        if (!Insertar(r)) {
            i--;
        }
    }
     
    pthread_t* manejadores_hilos;
    manejadores_hilos = malloc(cantidad_hilos * sizeof(pthread_t));
    pthread_mutex_init(&mutex_conteo, NULL);
    rwlock_init(&bloqueo_personalizado);

    tiempo_inicio = clock();
    for (i = 0; i < cantidad_hilos; i++)
        pthread_create(&manejadores_hilos[i], NULL, Funcion_Hilo, (void*) i);

    for (i = 0; i < cantidad_hilos; i++)
        pthread_join(manejadores_hilos[i], NULL);
         
    tiempo_fin = clock();
    tiempo_total = (tiempo_fin - tiempo_inicio) / CLOCKS_PER_SEC;
    printf("Tiempo transcurrido = %.10f segundos\n", tiempo_total);
    
    Limpiar_Memoria();
    pthread_mutex_destroy(&bloqueo_personalizado.mutex);
    pthread_cond_destroy(&bloqueo_personalizado.lectores);
    pthread_cond_destroy(&bloqueo_personalizado.escritores);
    pthread_mutex_destroy(&mutex_conteo);
    free(manejadores_hilos);

    return 0;
}

// Función Miembro
int Miembro(int valor) {
    struct nodo* temp = cabeza;
    while (temp != NULL && temp->dato < valor)
        temp = temp->siguiente;

    return (temp != NULL && temp->dato == valor);
}

// Función Insertar
int Insertar(int valor) {
    struct nodo* actual = cabeza;
    struct nodo* anterior = NULL;
    struct nodo* temp;
    int resultado = 1;

    while (actual != NULL && actual->dato < valor) {
        anterior = actual;
        actual = actual->siguiente;
    }

    if (actual == NULL || actual->dato > valor) {
        temp = malloc(sizeof(struct nodo));
        temp->dato = valor;
        temp->siguiente = actual;
        if (anterior == NULL)
            cabeza = temp;
        else
            anterior->siguiente = temp;
    } else {
        resultado = 0;
    }
    return resultado;
}

// Función Eliminar
int Eliminar(int valor) {
    struct nodo* actual = cabeza;
    struct nodo* anterior = NULL;
    int resultado = 1;

    while (actual != NULL && actual->dato < valor) {
        anterior = actual;
        actual = actual->siguiente;
    }

    if (actual != NULL && actual->dato == valor) {
        if (anterior == NULL) {
            cabeza = actual->siguiente;
            free(actual);
        } else {
            anterior->siguiente = actual->siguiente;
            free(actual);
        }
    } else {
        resultado = 0;
    }

    return resultado;
}

// Liberar memoria de la lista enlazada
void Limpiar_Memoria(void) {
    struct nodo* actual = cabeza;
    struct nodo* siguiente;
    
    while (actual != NULL) {
        siguiente = actual->siguiente;
        free(actual);
        actual = siguiente;
    }
}

// Verificar si la lista enlazada está vacía
int Esta_Vacia(void) {
    return (cabeza == NULL);
}

// Función que ejecutan los hilos
void* Funcion_Hilo(void* rango) {
    int i, valor;
    int mi_miembro = 0, mi_insertar = 0, mi_eliminar = 0;
    int ops_por_hilo = m / cantidad_hilos;

    for (i = 0; i < ops_por_hilo; i++) {
        float eleccion = (rand() % 10000 / 10000.0);
        valor = rand() % CLAVE_MAXIMA;

        if (eleccion < fracMiembro) {
            rwlock_lectura_bloquear(&bloqueo_personalizado);
            Miembro(valor);
            rwlock_lectura_desbloquear(&bloqueo_personalizado);
            mi_miembro++;
        } else if (eleccion < fracMiembro + fracInsertar) {
            rwlock_escritura_bloquear(&bloqueo_personalizado);
            Insertar(valor);
            rwlock_escritura_desbloquear(&bloqueo_personalizado);
            mi_insertar++;
        } else {
            rwlock_escritura_bloquear(&bloqueo_personalizado);
            Eliminar(valor);
            rwlock_escritura_desbloquear(&bloqueo_personalizado);
            mi_eliminar++;
        }
    }

    pthread_mutex_lock(&mutex_conteo);
    conteo_miembro += mi_miembro;
    conteo_insertar += mi_insertar;
    conteo_eliminar += mi_eliminar;
    pthread_mutex_unlock(&mutex_conteo);
    return NULL;
}
