#include <windows.h>
#include <stdio.h>
#include <string.h>

#define SIZE 4096
#define ITERATIONS 10000

double get_time_ms()
{
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart * 1000.0 / freq.QuadPart;
}

void test_shared_memory()
{

    HANDLE hMapFile;
    char *pBuf;

    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        SIZE,
        "Local\\IPC_TEST");

    pBuf = (char *)MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        SIZE);

    double start = get_time_ms();

    for (int i = 0; i < ITERATIONS; i++)
    {
        strcpy(pBuf, "Hello IPC");
    }

    double end = get_time_ms();

    printf("Memory Mapped File Time: %.4f ms\n", end - start);

    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
}

void test_named_pipe()
{

    HANDLE hPipeServer, hPipeClient;
    char buffer[SIZE] = "Hello IPC";
    DWORD written, read;

    // Create pipe server
    hPipeServer = CreateNamedPipe(
        "\\\\.\\pipe\\ipc_test",
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        1,
        SIZE,
        SIZE,
        0,
        NULL);

    // Create client connection
    hPipeClient = CreateFile(
        "\\\\.\\pipe\\ipc_test",
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    ConnectNamedPipe(hPipeServer, NULL);

    double start = get_time_ms();

    for (int i = 0; i < ITERATIONS; i++)
    {

        WriteFile(hPipeClient, buffer, strlen(buffer) + 1, &written, NULL);
        ReadFile(hPipeServer, buffer, SIZE, &read, NULL);
    }

    double end = get_time_ms();

    printf("Named Pipe Time: %.4f ms\n", end - start);

    CloseHandle(hPipeServer);
    CloseHandle(hPipeClient);
}

int main()
{

    printf("Windows IPC Benchmark\n\n");

    test_shared_memory();

    test_named_pipe();

    return 0;
}
