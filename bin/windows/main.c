#include <stdio.h>
#include <stdlib.h>
#include "region_explorer.h"


int belongs_region(double x, double y) {
    return x * x / 2 + y * y < 4;
}

int main(int argc, char* argv[]) {
    init_region_explorer(belongs_region, 1);
    start_region_explorer();
    
    return 0;
}
