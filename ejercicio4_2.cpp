#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <ctime>

using namespace std;

long long global_tiros_circulo = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct ThreadData {
    long long tirosPerThread;
    unsigned int seed;
    long long local_tiros_circulo;
};

void* monte_carlo_pi(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    data->local_tiros_circulo = 0;

    for (long long tiros = 0; tiros < data->tirosPerThread; tiros++) {
        double x = (rand_r(&data->seed) / (double)RAND_MAX) * 2.0 - 1.0;
        double y = (rand_r(&data->seed) / (double)RAND_MAX) * 2.0 - 1.0;
        double distCuadrado = x * x + y * y;

        if (distCuadrado <= 1) {
            data->local_tiros_circulo++;
        }
    }

    pthread_mutex_lock(&mutex);
    global_tiros_circulo += data->local_tiros_circulo;
    pthread_mutex_unlock(&mutex);

    pthread_exit(nullptr);
}

int main() {
    int num_threads;
    long long totalTiros;

    cout << "Ingrese el número total de lanzamientos: ";
    cin >> totalTiros;
    cout << "Ingrese el número de hilos: ";
    cin >> num_threads;

    long long tirosPerThread = totalTiros / num_threads;
    pthread_t threads[num_threads];
    ThreadData threadData[num_threads];

    for (int i = 0; i < num_threads; i++) {
        threadData[i].tirosPerThread = tirosPerThread;
        threadData[i].seed = time(0) + i;  
        pthread_create(&threads[i], nullptr, monte_carlo_pi, (void*)&threadData[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], nullptr);
    }

    double pi_estimate = 4.0 * global_tiros_circulo / static_cast<double>(totalTiros);
    cout << "La estimación de π es: " << pi_estimate << endl;

    pthread_mutex_destroy(&mutex);

    return 0;
}
