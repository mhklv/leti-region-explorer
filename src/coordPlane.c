#include "coordPlane.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


const static int AXIS_THICKNESS = 2;

static void draw_x_axis(CoordPlane *coord_plane);
static void draw_y_axis(CoordPlane *coord_plane);
static void draw_horizontal_line_and_label(CoordPlane *coord_plane, double t);
static void draw_vertical_line_and_label(CoordPlane *coord_plane, double t);
static void draw_line_segments(CoordPlane *coord_plane);
static void draw_points(CoordPlane *coord_plane);
static void get_label_points_between(double left, double right, double *label_points, int *points_n);
static void draw_horiz_gridlines(CoordPlane *coord_plane);
static void draw_vert_gridlines(CoordPlane *coord_plane);
static void fill_circle(SDL_Renderer *renderer, int x, int y, int r);
static void draw_region(CoordPlane *coord_plane);

// static void translate_c_to_v(double xc, double yc, int *xv, int *yv);
// static void translate_v_to_c(int xv, int yv, double *xc, double *yc);

    

void draw_coord_plane(CoordPlane *coord_plane) {
    // Render background.
    SDL_SetRenderDrawColor(coord_plane->renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(coord_plane->renderer, &coord_plane->pos_in_window);
    
    SDL_SetRenderDrawColor(coord_plane->renderer, 148, 222, 255, 255);
    // draw_points(coord_plane);
    draw_region(coord_plane);
    
    SDL_SetRenderDrawColor(coord_plane->renderer, coord_plane->line_color.r,
                           coord_plane->line_color.g,
                           coord_plane->line_color.b, 255);
    draw_x_axis(coord_plane);
    draw_y_axis(coord_plane);
    
    SDL_SetRenderDrawColor(coord_plane->renderer,
                           coord_plane->line_color.r,
                           coord_plane->line_color.g,
                           coord_plane->line_color.b, 80);

    draw_horiz_gridlines(coord_plane);
    draw_vert_gridlines(coord_plane);
}


int coord_plane_process_event(CoordPlane *coord_plane, SDL_Event *event) {
    if (event->type == SDL_MOUSEMOTION) {
        SDL_MouseMotionEvent *m_event = &event->motion;
        double dx, dy;
        
        if (m_event->state & SDL_BUTTON_LMASK) {
            dy = m_event->yrel * coord_plane->viewport.h / coord_plane->pos_in_window.h;
            dx = m_event->xrel * coord_plane->viewport.w / coord_plane->pos_in_window.w;
            
            coord_plane->viewport.left -= dx;
            coord_plane->viewport.bot += dy;
            
            return 1;
        }
    }
    else if (event->type == SDL_MOUSEWHEEL) {
        SDL_MouseWheelEvent *mw_event = &event->wheel;
        int msx, msy;
        double f, mvx, mvy;
        double b = coord_plane->viewport.bot, hc = coord_plane->viewport.h;
        double hv = coord_plane->pos_in_window.h, py = coord_plane->pos_in_window.y;
        double l = coord_plane->viewport.left, wc = coord_plane->viewport.w;
        double wv = coord_plane->pos_in_window.w, px = coord_plane->pos_in_window.x;
        
        if (mw_event->y > 0) {
            f = 0.9;
        }
        else {
            f = 1.0 / 0.9;
        }

        SDL_GetMouseState(&msx, &msy);
        mvy = b + hc * (py + hv - msy) / hv;
        mvx = l + wc * (msx - px) / wv;

        coord_plane->viewport.left = mvx + f * (l - mvx);
        coord_plane->viewport.w = f * wc;
        coord_plane->viewport.bot = mvy + f * (b - mvy);
        coord_plane->viewport.h = f * hc;
        
        return 1;
    }
    else if (event->type == SDL_MOUSEBUTTONDOWN) {
        if (event->button.button == SDL_BUTTON_LEFT) {
            SDL_CaptureMouse(SDL_TRUE);

            return 1;
        }
    }
    else if (event->type == SDL_MOUSEBUTTONUP) {
        if (event->button.button == SDL_BUTTON_LEFT) {
            SDL_CaptureMouse(SDL_FALSE);
            return 1;
        }
    }
    
    return 0;
}

static void draw_region(CoordPlane *coord_plane) {
    static SDL_Rect rects[800*800];
    double b = coord_plane->viewport.bot, hc = coord_plane->viewport.h;
    double hv = coord_plane->pos_in_window.h, py = coord_plane->pos_in_window.y;
    double l = coord_plane->viewport.left, wc = coord_plane->viewport.w;
    double wv = coord_plane->pos_in_window.w, px = coord_plane->pos_in_window.x;
    double xc, yc;
    int step = coord_plane->region_granularity, n = 0;

    for (int yv = py; yv < py + hv; yv += step) {
        yc = b + hc * (py + hv - yv) / hv;
        for (int xv = px; xv < px + wv; xv += step) {
            xc = l + wc * (xv - px) / wv;
            if (coord_plane->belongs_region(xc, yc)) {
                rects[n].x = xv - step / 2, rects[n].y = yv - step / 2, rects[n].w = rects[n].h = step;
                ++n;
            }
        }
    }
    SDL_RenderFillRects(coord_plane->renderer, rects, n);
}

static void draw_x_axis(CoordPlane *coord_plane) {
    Coord_Rect *coord_viewport = &coord_plane->viewport;

    if (coord_viewport->bot >= 0 || coord_viewport->bot <= -coord_viewport->h) {
        // No x axis in the viewport.
        return;
    }
        
    int x1, y1, x2, y2;

    y1 = y2 = (int) (coord_viewport->bot * coord_plane->pos_in_window.h / coord_viewport->h) +
        coord_plane->pos_in_window.y + coord_plane->pos_in_window.h;
    x1 = coord_plane->pos_in_window.x;
    x2 = coord_plane->pos_in_window.x + coord_plane->pos_in_window.w;

    SDL_Rect axis_rect;
    axis_rect.y = y1 - AXIS_THICKNESS / 2;
    axis_rect.x = x1;
    axis_rect.h = AXIS_THICKNESS;
    axis_rect.w = x2 - x1;
    SDL_RenderFillRect(coord_plane->renderer, &axis_rect);
}

static void draw_y_axis(CoordPlane *coord_plane) {
    Coord_Rect *coord_viewport = &coord_plane->viewport;

    if (coord_viewport->left >= 0 || coord_viewport->left <= -coord_viewport->w) {
        // No y axis in the viewport.
        return;
    }
        
    int x1, y1, x2, y2;

    x1 = x2 = -(int) (coord_viewport->left * coord_plane->pos_in_window.w / coord_viewport->w) +
        coord_plane->pos_in_window.x;
    y1 = coord_plane->pos_in_window.y;
    y2 = coord_plane->pos_in_window.y + coord_plane->pos_in_window.h;

    SDL_Rect axis_rect;
    axis_rect.y = y1;
    axis_rect.x = x1 - AXIS_THICKNESS / 2;
    axis_rect.h = y2 - y1;
    axis_rect.w = AXIS_THICKNESS;
    SDL_RenderFillRect(coord_plane->renderer, &axis_rect);
}

static void draw_horizontal_line_and_label(CoordPlane *coord_plane, double t) {
    static char label_str[128];
    int text_w, text_h;
    int x1, y1, x2, y2;
    double b = coord_plane->viewport.bot, hc = coord_plane->viewport.h;
    double hv = coord_plane->pos_in_window.h, py = coord_plane->pos_in_window.y;
    double l = coord_plane->viewport.left, wc = coord_plane->viewport.w;
    double wv = coord_plane->pos_in_window.w, px = coord_plane->pos_in_window.x;

    y1 = y2 = hv * (b - t) / hc + py + hv;
    x1 = coord_plane->pos_in_window.x;
    x2 = coord_plane->pos_in_window.x + coord_plane->pos_in_window.w;
    SDL_RenderDrawLine(coord_plane->renderer, x1, y1, x2, y2);

    
    // Было бы эффективнее переиспользовать одну и ту же текстуру в
    // разных кадрах, вместо того, чтобы создавать каждый раз
    // новую. Но так тоже нормально.
    snprintf(label_str, 128, "%.3g", (fabs(t) < 1e-8) ? 0.0 : t);
    int label_x = -(int) (l * wv / wc) + px;
    if (fabs(t) < 1e-8) {
        y1 += 15;
    }
    TTF_SizeText(coord_plane->label_font, label_str, &text_w, &text_h);
    label_x = (label_x < 0) ? 5 : label_x + 5;
    label_x = (label_x + text_w > wv - 5) ? wv - text_w - 5 : label_x;
    // Warning: potential nullptr exception. Needs error handling.
    SDL_Surface *text_surface = TTF_RenderText_Blended(coord_plane->label_font, label_str, coord_plane->line_color);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(coord_plane->renderer, text_surface);
    SDL_RenderCopy(coord_plane->renderer, text_texture, NULL, &(SDL_Rect) {.x = label_x, .y = y1 - text_h / 2, .w = text_w, .h = text_h});
    SDL_FreeSurface(text_surface);
    SDL_DestroyTexture(text_texture);
}

static void draw_vertical_line_and_label(CoordPlane *coord_plane, double t) {
    static char label_str[128];
    int text_w, text_h;
    int x1, y1, x2, y2;
    double b = coord_plane->viewport.bot, hc = coord_plane->viewport.h;
    double hv = coord_plane->pos_in_window.h, py = coord_plane->pos_in_window.y;
    double l = coord_plane->viewport.left, wc = coord_plane->viewport.w;
    double wv = coord_plane->pos_in_window.w, px = coord_plane->pos_in_window.x;

    x1 = x2 = wv * (t - l) / wc + px;
    y1 = coord_plane->pos_in_window.y;
    y2 = coord_plane->pos_in_window.y + coord_plane->pos_in_window.h;
    SDL_RenderDrawLine(coord_plane->renderer, x1, y1, x2, y2);

    
    if (fabs(t) < 1e-8) {
        return;
    }
    snprintf(label_str, 128, "%.3g", (fabs(t) < 1e-8) ? 0.0 : t);
    int label_y = (int) (b * hv / hc) + py + hv;
    TTF_SizeText(coord_plane->label_font, label_str, &text_w, &text_h);
    label_y = (label_y < 0) ? 5 : label_y + 5;
    label_y = (label_y + text_h > hv - 5) ? hv - text_h - 5 : label_y;
    // Warning: potential nullptr exception. Needs error handling.
    SDL_Surface *text_surface = TTF_RenderText_Blended(coord_plane->label_font, label_str, coord_plane->line_color);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(coord_plane->renderer, text_surface);
    SDL_RenderCopy(coord_plane->renderer, text_texture, NULL, &(SDL_Rect) {.x = x1 - text_w / 2, .y = label_y, .w = text_w, .h = text_h});
    SDL_FreeSurface(text_surface);
    SDL_DestroyTexture(text_texture);
}

// Bresenham's circle drawing algorithm.
static void fill_circle(SDL_Renderer *renderer, int x, int y, int r) {
    int xi = r, yi = 0, d = 1 - r;
    static SDL_Rect rects[4];

    while (yi <= xi) {
        // Can't use RenderDrawLine because of some bugs in it (probably):
        // https://bugzilla.libsdl.org/show_bug.cgi?id=5050

        rects[0].x = x - xi, rects[0].y = y + yi, rects[0].w = 2*xi + 1, rects[0].h = 1;
        rects[1].x = x - xi, rects[1].y = y - yi, rects[1].w = 2*xi + 1, rects[1].h = 1;
        rects[2].x = x - yi, rects[2].y = y + xi, rects[2].w = 2*yi + 1, rects[2].h = 1;
        rects[3].x = x - yi, rects[3].y = y - xi, rects[3].w = 2*yi + 1, rects[3].h = 1;
        SDL_RenderFillRects(renderer, rects, 4);
        
        yi += 1;
        if (d <= 0) {
            d += 2 * yi + 1;
        }
        else {
            xi -= 1;
            d += 2 * (yi - xi) + 1;
        }
    }
}

static void draw_line_segments(CoordPlane *coord_plane) {
    double b = coord_plane->viewport.bot, hc = coord_plane->viewport.h;
    double hv = coord_plane->pos_in_window.h, py = coord_plane->pos_in_window.y;
    double l = coord_plane->viewport.left, wc = coord_plane->viewport.w;
    double wv = coord_plane->pos_in_window.w, px = coord_plane->pos_in_window.x;
    int xv1, yv1, xv2, yv2;

    for (int i = 0; i < coord_plane->line_seg_n; ++i) {
        xv1 = wv * (coord_plane->line_segments[i].x1 - l) / wc + px;
        yv1 = hv * (b - coord_plane->line_segments[i].y1) / hc + py + hv;
        xv2 = wv * (coord_plane->line_segments[i].x2 - l) / wc + px;
        yv2 = hv * (b - coord_plane->line_segments[i].y2) / hc + py + hv;

        fill_circle(coord_plane->renderer, xv1, yv1, 3);
        fill_circle(coord_plane->renderer, xv2, yv2, 3);
        SDL_RenderDrawLine(coord_plane->renderer, xv1, yv1, xv2, yv2);
    }
}

static void draw_points(CoordPlane *coord_plane) {
    double b = coord_plane->viewport.bot, hc = coord_plane->viewport.h;
    double hv = coord_plane->pos_in_window.h, py = coord_plane->pos_in_window.y;
    double l = coord_plane->viewport.left, wc = coord_plane->viewport.w;
    double wv = coord_plane->pos_in_window.w, px = coord_plane->pos_in_window.x;
    int xv, yv, pr;
    double prx, pry;

    for (int i = 0; i < coord_plane->points_n; ++i) {
        xv = wv * (coord_plane->points[i].x - l) / wc + px;
        yv = hv * (b - coord_plane->points[i].y) / hc + py + hv;

        if (isinf(coord_plane->points[i].closest_point_rx) ||
            isinf(coord_plane->points[i].closest_point_ry)) {
            pr = 8;
        }
        else {
            prx = wv * coord_plane->points[i].closest_point_rx / wc;
            pry = hv * coord_plane->points[i].closest_point_ry / hc;
            pr = round(sqrt(prx*prx + pry*pry)) / 2 - 2;
        }
        pr = (pr < 1) ? 1 : pr;
        pr = (pr > 8) ? 8 : pr;

        if (xv + pr >= px && xv - pr <= px + wv && yv + pr >= py && yv - pr <= py + hv) {
            fill_circle(coord_plane->renderer, xv, yv, pr);
        }
    }
}

// Long labels might intersect sometimes. This needs improvements.
static void get_label_points_between(double left, double right, double *label_points, int *points_n) {
    double n, a = 10, b = 0, d1, delta;

    b = floor(log(right - left) / log(a) - 0.15);
    delta = pow(a, b);
    n = ceil((right - left) / delta);
    d1 = delta - left + floor(left / delta) * delta;
    *points_n = n;
    
    label_points[0] = left + d1;
    for (int i = 1; i < *points_n; ++i) {
        label_points[i] = label_points[i - 1] + delta;
    }
}

static void draw_horiz_gridlines(CoordPlane *coord_plane) {
    static double label_points[128];
    static char label_str[128];
    int n;
    double left = coord_plane->viewport.bot;
    double right = coord_plane->viewport.bot + coord_plane->viewport.h;

    get_label_points_between(left, right, label_points, &n);

    for (int i = 0; i < n; ++i) {
        int text_w, text_h;
        draw_horizontal_line_and_label(coord_plane, label_points[i]);
    }

}

static void draw_vert_gridlines(CoordPlane *coord_plane) {
    static double label_points[100];
    int n;
    double left = coord_plane->viewport.left;
    double right = coord_plane->viewport.left + coord_plane->viewport.w;

    get_label_points_between(left, right, label_points, &n);

    for (int i = 0; i < n; ++i) {
        draw_vertical_line_and_label(coord_plane, label_points[i]);
    }
}
