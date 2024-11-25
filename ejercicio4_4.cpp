#include <iostream>
#include <pthread.h>
#include <ctime>

using namespace std;

void* funcion_hilo(void* arg) {
    pthread_exit(nullptr);
}

int main() {
    int num_hilos;
    cout << "Ingrese el número de hilos a crear: ";
    cin >> num_hilos;

    if (num_hilos <= 0) {
        cout << "El número de hilos debe ser mayor a 0." << endl;
        return EXIT_FAILURE;
    }

    pthread_t hilo;
    struct timespec inicio, fin;
    long long tiempo_total_ns = 0;

    for (int i = 0; i < num_hilos; i++) {
        clock_gettime(CLOCK_MONOTONIC, &inicio);

        pthread_create(&hilo, nullptr, funcion_hilo, nullptr);
        pthread_join(hilo, nullptr);

        clock_gettime(CLOCK_MONOTONIC, &fin);

        long long tiempo_ns = (fin.tv_sec - inicio.tv_sec) * 1e9 + (fin.tv_nsec - inicio.tv_nsec);
        tiempo_total_ns += tiempo_ns;
    }

    double tiempo_promedio_ns = static_cast<double>(tiempo_total_ns) / num_hilos;

    cout << "Tiempo promedio para crear y terminar un hilo: " << tiempo_promedio_ns << " ns" << endl;

    return EXIT_SUCCESS;
}