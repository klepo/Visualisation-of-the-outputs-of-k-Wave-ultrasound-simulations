#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iostream>
#include <string>

#ifdef __unix
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#endif

#ifdef _WIN32
#include <Windows.h>
#include <winsock.h>
#endif

#include <omp.h>

//#include <k-wave-h5-helper.h>

#ifdef _WIN32
void *memalloc(size_t alignment, size_t size)
{
    return _aligned_malloc(size, alignment);
}

void memfree(void *data)
{
    _aligned_free(data);
}
#else
void *memalloc(size_t alignment, size_t size)
{
    return aligned_alloc(alignment, size);
}

void memfree(void *data)
{
    free(data);
}
#endif

size_t getTotalSystemPhysicalMemory()
{
#ifdef __unix
    long pages     = sysconf(_SC_PHYS_PAGES);
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
    long pages     = sysconf(_SC_AVPHYS_PAGES);
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

size_t roundUp(size_t numToRound, size_t multiple)
{
    if (multiple == 0) {
        return numToRound;
    }

    size_t remainder = numToRound % multiple;
    if (remainder == 0)
        return numToRound;
    return numToRound + multiple - remainder;
}

int main(int argc, char **argv)
{
    float coeff = 0.9999f;

    if (argc == 2) {
        try {
            size_t pos;
            coeff = std::stof(argv[1], &pos);
            if (strlen(argv[1]) != pos || coeff <= 0 || coeff >= 1)
                throw std::invalid_argument(argv[1]);
        } catch (std::invalid_argument) {
            printf("Wrong parameter");
            exit(EXIT_FAILURE);
        }
    }

    // mlockall(MCL_CURRENT | MCL_FUTURE);

    // setlocale(LC_NUMERIC, "");

    std::cout << "Total system physical memory:                       " << getTotalSystemPhysicalMemory() << " bytes"
              << std::endl;
    std::cout << "Available system physical memory before allocation: " << getAvailableSystemPhysicalMemory()
              << " bytes" << std::endl;
    size_t size = size_t(float(getAvailableSystemPhysicalMemory()) * coeff);
    std::cout << "Memory to allocate:                                 " << size << " bytes" << std::endl;
    char *mem = static_cast<char *>(memalloc(upperPowerOfTwo(1000000000), size));
    std::cout << "Allocation is running ..." << std::endl;
#pragma omp parallel for
    for (long long i = 0; i < static_cast<long long>(size); i++) {
        mem[i] = 0; //'A' + rand() % 24;
    }
    std::cout << "Available system physical memory after allocation:  " << getAvailableSystemPhysicalMemory()
              << " bytes" << std::endl;
    memfree(mem);
    std::cout << "Available system physical memory after freeing:     " << getAvailableSystemPhysicalMemory()
              << " bytes" << std::endl;

    // pripravit soubory, ruzne chunky
    // linearni,
    // bloky 1x8x8x8,    2x8x8x8,    4x8x8x8,    8x8x8x8,    16x8x8x8,    32x8x8x8,    64x8x8x8
    // bloky 1x16x16x16, 2x16x16x16, 4x16x16x16, 8x16x16x16, 16x16x16x16, 32x16x16x16, 16x16x16x16
    // bloky 1x32x32x32, 2x32x32x32, 4x32x32x32, 8x32x32x32, 16x32x32x32, 32x32x32x32, 32x32x32x32
    // bloky 1x64x64x64, 2x64x64x64, 4x64x64x64, 8x64x64x64, 16x64x64x64, 32x64x64x64, 64x64x64x64
    // udelat rozlozeni manualne?

    /*try {
        H5Helper::File *file = new H5Helper::File("d:/study/DVI/data/linear/output_kidney_linear_p_cuboid_modified.h5",
    H5Helper::File::OPEN, true); H5Helper::Dataset *dataset = file->openDataset("p/0", true); float *dataXY = nullptr;
        float *dataXZ = nullptr;
        float *dataYZ = nullptr;
        H5Helper::Vector4D dims = dataset->getDims();
        std::cout << "Dimensions \t" << dims << std::endl;
        std::cout << "Chunks     \t" << dataset->getChunkDims() << std::endl;
        dataset->readDataset(H5Helper::Vector4D(dims.w() / 2, dims.z() / 2, 0, 0), H5Helper::Vector4D(1, 1, dims.y(),
    dims.x()), dataXY, true); dataset->readDataset(H5Helper::Vector4D(dims.w() / 2, 0, dims.y() / 2, 0),
    H5Helper::Vector4D(1, dims.z(), 1, dims.x()), dataXZ, true); dataset->readDataset(H5Helper::Vector4D(dims.w() / 2,
    0, 0, dims.x() / 2), H5Helper::Vector4D(1, dims.z(), dims.y(), 1), dataYZ, true); delete[] dataXY; delete[] dataXZ;
        delete[] dataYZ;
        delete file;
    } catch (std::exception &e) {
           std::cerr << e.what() << std::endl;
           std::exit(EXIT_FAILURE);
    }*/

    /*int *mem = nullptr;
    size_t size = 0;
    size_t block = 1024 * 1024 * 32;
    size_t inc = block * sizeof(int);
    size_t min = block * 16;
    if (argc == 2) {
        try {
            size_t pos;
            min = std::stoull(argv[1], &pos);
            if (strlen(argv[1]) != pos || min <= 0)
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
        if (mem == nullptr || min > getAvailableSystemPhysicalMemory())
            break;
        size += inc;
        printf("Allocated size:                     \t%'zu bytes\n", size);
        printf("Available system physical memory:   \t%'zu bytes\n", getAvailableSystemPhysicalMemory());
        printf("--------\n");
        for (unsigned int i = 0; i < block; i++) {
            mem[i] = 1;
        }
    } while (mem != nullptr && min < getAvailableSystemPhysicalMemory());
    if (mem == nullptr)
        printf("mem == NULL\n");
    printf("--------\n");
    printf("Available system physical memory: \t%'zu bytes\n", getAvailableSystemPhysicalMemory());
    printf("Allocated size:                   \t%'zu bytes\n", size);
*/
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
