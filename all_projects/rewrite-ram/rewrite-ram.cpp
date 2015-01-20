#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <mpi.h>

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

    printf("Total system physical memory:     \t%'zu \n", getTotalSystemPhysicalMemory());
    printf("Available system physical memory: \t%'zu \n", getAvailableSystemPhysicalMemory());

    int *mem = NULL;
    size_t size = 0;
    size_t block = 1024 * 1024 * 32;
    size_t inc = block * sizeof(int);
    size_t min = block * 16;
    printf("Allocation is running ... \n");
    do {
        mem = (int *) calloc(1, inc);
        if (mem == NULL || min > getAvailableSystemPhysicalMemory())
            break;
        size += inc;
        printf("size: \t%'zu \n", size);
        printf("getAvailableSystemPhysicalMemory(): \t%'zu \n", getAvailableSystemPhysicalMemory());
        for (unsigned int i = 0; i < block; i++) {
            mem[i] = 1;
        }
    } while (mem != NULL && min < getAvailableSystemPhysicalMemory());
    if (mem == NULL)
        printf("mem == NULL\n");
    printf("Available system physical memory: \t%'zu \n", getAvailableSystemPhysicalMemory());
    printf("Allocated size:                   \t%'zu \n", size);

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
