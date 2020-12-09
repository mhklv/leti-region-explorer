#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "coordPlane.h"
#include "region_explorer.h"


const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 800;
static int g_granularity;
static int (*g_belongs_region)(double, double);


void init_region_explorer(int (*belongs_region)(double, double), int granularity) {
    g_granularity = granularity;
    g_belongs_region = belongs_region;
}

void init_coord_plane(CoordPlane *coord_plane) {
    coord_plane->viewport.left = -5;
    coord_plane->viewport.bot = -5;
    coord_plane->viewport.w = 10;
    coord_plane->viewport.h = 10;
    
    SDL_Color line_color;
    line_color.r = 0;
    line_color.g = 0;
    line_color.b = 0;
    line_color.a = 255;
    coord_plane->line_color = line_color;
    
    SDL_Rect coord_plane_rect;
    coord_plane_rect.x = 0;
    coord_plane_rect.y = 0;
    coord_plane_rect.w = WINDOW_WIDTH;
    coord_plane_rect.h = WINDOW_HEIGHT;
    coord_plane->pos_in_window = coord_plane_rect;

    TTF_Font *font;
    font = TTF_OpenFont("res/font.ttf", 22);
    coord_plane->label_font = font;
    
    coord_plane->belongs_region = g_belongs_region;
    coord_plane->region_granularity = g_granularity;
}

int start_region_explorer() {
    SDL_SetMainReady();
    SDL_Window *main_window = NULL;
    SDL_Renderer *main_renderer = NULL;
    SDL_Surface *main_surface = NULL;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "%s\n", SDL_GetError());
        fprintf(stderr, "Failed to initialize SDL. Exiting...\n");
        return 1;
    }
    
    if (TTF_Init() != 0) {
        fprintf(stderr, "%s\n", TTF_GetError());
        fprintf(stderr, "Failed to initialize SDL_TTF. Exiting...\n");
        return 1;
    }
    
    main_window = SDL_CreateWindow("График", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    
    if (main_window == NULL) {
        fprintf(stderr, "%s\n", SDL_GetError());
        fprintf(stderr, "Failed to create window. Exiting...\n");
        return 1;
    }
    
    main_renderer = SDL_CreateRenderer(main_window, -1, 0);
    SDL_SetRenderDrawBlendMode(main_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(main_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(main_renderer);
    SDL_RenderPresent(main_renderer);
    
    CoordPlane coord_plane;
    init_coord_plane(&coord_plane);
    coord_plane.renderer = main_renderer;

    SDL_Event event;
    int is_exited = 0, coord_plane_changed = 0;
    SDL_RenderClear(coord_plane.renderer);
    draw_coord_plane(&coord_plane);
    SDL_RenderPresent(coord_plane.renderer);
    
    while (!is_exited) {
        if (SDL_WaitEvent(&event) == 0) {
            return 1;
        }
        if (event.type == SDL_QUIT) {
            is_exited = 1;
        }
        
        coord_plane_changed |= coord_plane_process_event(&coord_plane, &event);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_exited = 1;
            }
            coord_plane_changed |= coord_plane_process_event(&coord_plane, &event);
        }

        if (coord_plane_changed) {
            SDL_RenderClear(coord_plane.renderer);
            draw_coord_plane(&coord_plane);
            SDL_RenderPresent(coord_plane.renderer);
        }
        
        coord_plane_changed = 0;
    }

    SDL_DestroyRenderer(main_renderer);
    SDL_DestroyWindow(main_window);

    SDL_Quit();
    TTF_Quit();
    
    return 0;
}
