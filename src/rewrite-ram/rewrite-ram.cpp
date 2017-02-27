#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <mpi.h>
#include <iostream>
#include <string>

#ifdef __unix
#include <unistd.h>
#include <sys/mman.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <Winsock2.h>
#endif

size_t getTotalSystemPhysicalMemory()
{
    #ifdef __unix
        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);
        return pages * page_size;
    #endif

    #ifdef _WIN32
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        GlobalMemoryStatusEx(&status);
        return status.ullTotalPhys;
    #endif
}

size_t getAvailableSystemPhysicalMemory()
{
    #ifdef __unix
        long pages = sysconf(_SC_AVPHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);
        return pages * page_size;
    #endif

    #ifdef _WIN32
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        GlobalMemoryStatusEx(&status);
        return status.ullAvailPhys;
    #endif
}

size_t upperPowerOfTwo(size_t x)
{
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;

}

size_t roundUp(size_t numToRound, int multiple)
{
    if (multiple == 0) {
        return numToRound;
    }

    int remainder = numToRound % multiple;
    if (remainder == 0)
        return numToRound;
    return numToRound + multiple - remainder;
}

int mPISize;
int mPIRank;
MPI_Comm comm = MPI_COMM_WORLD;
MPI_Info info = MPI_INFO_NULL;

int main(int argc, char **argv) {
    mlockall(MCL_CURRENT | MCL_FUTURE);

    int buflen = 512;
    char hostname[buflen];
    gethostname(hostname, buflen);
    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &mPISize);
    MPI_Comm_rank(comm, &mPIRank);

    std::cout << "process rank " << mPIRank << " of " << mPISize << " on host " << hostname << std::endl;

    setlocale(LC_NUMERIC, "");

    printf("Total system physical memory:     \t%'zu bytes\n", getTotalSystemPhysicalMemory());
    printf("Available system physical memory: \t%'zu bytes\n", getAvailableSystemPhysicalMemory());

    int *mem = NULL;
    size_t size = 0;
    size_t block = 1024 * 1024 * 32;
    size_t inc = block * sizeof(int);
    size_t min = block * 16;
    if (argc == 2) {
        try {
            size_t s;
            min = std::stoi(argv[1], &s);
            if (strlen(argv[1]) != s || min <= 0)
                throw std::invalid_argument(argv[1]);
            //min = roundUp(min, 2);
            //min = roundUp(min, 8);
            printf("Minimal memory:                   \t%'zu bytes\n", min);
        } catch (std::invalid_argument) {
            printf("Wrong parameter");
            exit(EXIT_FAILURE);
        }
    }

    printf("--------\n");
    printf("Allocation is running ... \n");
    printf("--------\n");
    do {
        mem = (int *) calloc(1, inc);
        if (mem == NULL || min > getAvailableSystemPhysicalMemory())
            break;
        size += inc;
        printf("Allocated size:                     \t%'zu bytes\n", size);
        printf("Available system physical memory:   \t%'zu bytes\n", getAvailableSystemPhysicalMemory());
        printf("--------\n");
        for (unsigned int i = 0; i < block; i++) {
            mem[i] = 1;
        }
    } while (mem != NULL && min < getAvailableSystemPhysicalMemory());
    if (mem == NULL)
        printf("mem == NULL\n");
    printf("--------\n");
    printf("Available system physical memory: \t%'zu bytes\n", getAvailableSystemPhysicalMemory());
    printf("Allocated size:                   \t%'zu bytes\n", size);

    MPI_Finalize();

    return 0;
}


/*
int main() {
    int *p;
    while(1) {
        int inc = 1024 * 1024 * sizeof(float);
        p = (int*) calloc(1, inc);
        if(!p) break;
    }
}*/
