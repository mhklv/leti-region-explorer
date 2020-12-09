#ifndef REGION_EXPLORER
#define REGION_EXPLORER


// Arguments :
//
// 1 -- pointer to a function that returns 1 if point (x, y) belongs
// given region. Else returns 0.
// 
// 2 (granularity) -- integer greater than 0. the less it is, the more
// precisely the region is displayed.
void init_region_explorer(int (*belongs_region)(double, double), int granularity);

int start_region_explorer();
    


#endif // REGION_EXPLORER
