#ifndef COORD_PLANE
#define COORD_PLANE

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"


typedef struct {
    double x, y;
    double closest_point_rx, closest_point_ry;
} Coord_Point;


typedef struct {
    double left, bot, w, h;
} Coord_Rect;


typedef struct {
    double x1, y1, x2, y2;
} Coord_LineSegment;


typedef struct {
    Coord_Rect viewport;
    Coord_Point *points;
    int points_n;
    Coord_LineSegment *line_segments;
    int line_seg_n;
    SDL_Color line_color;
    SDL_Renderer *renderer;
    SDL_Rect pos_in_window;
    TTF_Font *label_font;
    int (*belongs_region) (double, double);
    int region_granularity;
} CoordPlane;


void draw_coord_plane(CoordPlane *coord_plane);


// Returns :
// 0  -- if event was consumed.
// -1 -- if event wasn't intended for the coordinate plane.
int coord_plane_process_event(CoordPlane *coord_plane, SDL_Event *event);



#endif // COORD_PLANE
