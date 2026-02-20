#include <windows.h>
#include <stdio.h>

#define MAX_THREADS 16
#define ITERATIONS 1000000

HANDLE mutex;
long counter = 0;

DWORD WINAPI worker(LPVOID arg)
{

    for (int i = 0; i < ITERATIONS; i++)
    {
        WaitForSingleObject(mutex, INFINITE);
        counter++;
        ReleaseMutex(mutex);
    }
    return 0;
}

double get_time_sec()
{
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / freq.QuadPart;
}

int main()
{

    HANDLE threads[MAX_THREADS];
    mutex = CreateMutex(NULL, FALSE, NULL);

    printf("Windows Mutex Benchmark\n\n");

    for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2)
    {

        counter = 0;
        double start = get_time_sec();

        for (int i = 0; i < num_threads; i++)
            threads[i] = CreateThread(NULL, 0, worker, NULL, 0, NULL);

        WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);

        double end = get_time_sec();

        double time_taken = end - start;
        double throughput = counter / time_taken;

        printf("Threads: %d | Throughput: %.0f ops/sec\n", num_threads, throughput);
    }

    CloseHandle(mutex);

    return 0;
}
