#include <iostream>
#include <pthread.h>
#include <chrono>
#include <vector>

void* thread_function(void* arg) {
    return nullptr;
}

int main() {
    int num_threads;
    std::cout << "Ingrese el número de hilos: ";
    std::cin >> num_threads;

    std::vector<double> time_per_thread(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        auto start_time = std::chrono::high_resolution_clock::now();

        pthread_t thread;
        if (pthread_create(&thread, nullptr, thread_function, nullptr) != 0) {
            std::cerr << "Error al crear el hilo\n";
            return 1;
        }

        pthread_join(thread, nullptr);

        auto end_time = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::micro> duration = end_time - start_time;
        time_per_thread[i] = duration.count();
    }

    double total_time = 0;
    for (double t : time_per_thread) {
        total_time += t;
    }
    double average_time = total_time / num_threads;

    std::cout << "Tiempo promedio para crear y terminar un hilo: " << average_time << " microsegundos\n";

    return 0;
}
