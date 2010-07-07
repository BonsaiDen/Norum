#include <SDL.h>
#include "macro.h"


struct List;

struct Screen {
    SDL_Surface *bg;
    SDL_PixelFormat *format;
    int w, h;
    int scale;
    bool has_mouse, has_focus;
};

struct TileMap {
    SDL_Surface *img;
    int xs, ys;
    int h, w;
};

struct Engine {
    struct Screen *screen;
    bool running;
    bool paused;
    
    struct List *list_surfaces;
    struct List *list_tilemaps;
    
    int fps_rate;
    int fps_current;
    int fps_wait;
    int time;
    
    int keys[322];
    int key_count;
    
    int mouse[16];
    int mouse_x;
    int mouse_y;
    int mouse_count;
};


// Engine
int engine_create(const int width, const int height, const int scale, const int fps);
void engine_quit();

// Gets & Sets
int time_get();
int fps_get(const bool current);
void fps_set(const int fps);

// Input
bool key_down(const SDLKey key);
bool key_pressed(const SDLKey key);
int key_count();

bool mouse_down(const int key);
bool mouse_pressed(const int key);
bool mouse_on_screen();
void mouse_get_pos(int *x, int *y);

// Images
SDL_Surface *image_create(const int w, const int h, const int key);
SDL_Surface *image_load(const char *file, const int key);
void image_draw(SDL_Surface *bg, SDL_Surface *img, const int x, const int y);

// Tiles
struct TileMap *tiles_from_image(SDL_Surface *img, const int x, const int y);
struct TileMap *tiles_from_file(const char *file, const int x, const int y, const int key);
void tiles_draw(const struct TileMap *map, SDL_Surface *bg, const int index, int x, const int y);

// Drawing
int color_create(const int r, const int g, const int b);
int color_create_alpha(const int r, const int g, const int b, const int a);
void draw_rect(SDL_Surface *bg, const int x, const int y, const int w, const int h, const int color);
void draw_rect_filled(SDL_Surface *bg, const int x, const int y, const int w, const int h, const int color);

// Screen
struct Screen *screen_get();

