#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define SIZE 4096
#define ITERATIONS 10000

double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000.0) + (ts.tv_nsec / 1000000.0);
}

void test_shared_memory() {
    const char *name = "/ipc_test";
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SIZE);

    char *ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

    double start = get_time_ms();

    for(int i=0;i<ITERATIONS;i++) {
        strcpy(ptr, "Hello IPC");
    }

    double end = get_time_ms();

    printf("Shared Memory Time: %.4f ms\n", end - start);

    munmap(ptr, SIZE);
    close(shm_fd);
    shm_unlink(name);
}

void test_pipe() {
    int fd[2];
    pipe(fd);

    char buffer[SIZE] = "Hello IPC";

    double start = get_time_ms();

    for(int i=0;i<ITERATIONS;i++) {
        write(fd[1], buffer, strlen(buffer)+1);
        read(fd[0], buffer, SIZE);
    }

    double end = get_time_ms();

    printf("Pipe Time: %.4f ms\n", end - start);

    close(fd[0]);
    close(fd[1]);
}

int main() {

    printf("Linux IPC Benchmark\n");

    test_shared_memory();
    test_pipe();

    return 0;
}
