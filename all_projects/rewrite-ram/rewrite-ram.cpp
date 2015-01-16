#include <stdlib.h>

int main() {
    int *p;
    while(1) {
        int inc = 1024 * 1024 * sizeof(float);
        p = (int*) calloc(1, inc);
        if(!p) break;
    }
}
