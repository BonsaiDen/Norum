#include "engine.h"
#include "game.h"
#include "list.h"

#include <SDL_image.h>
#include <SDL.h>

#include <stdlib.h>
#include <stdio.h>


struct Engine *engine;

// Methods
bool engine_init(int width, int height, int scale, int fps);
void engine_update();
void engine_render(struct Screen *screen);
void engine_cleanup();
void engine_crash(const char *reason);

void engine_pause(bool mode);

void engine_events();
void engine_clear_keys();
void engine_clear_mouse();

void image_color_key(SDL_Surface *img, long int key);

struct Screen *screen_create(int width, int height, int scale);


// Engine ----------------------------------------------------------------------
// -----------------------------------------------------------------------------
int engine_create(int width, int height, int scale, int fps) {
    engine = malloc(sizeof(struct Engine));
    
    if (!engine_init(width, height, scale, fps)) {
        free(engine);
        return 1;
    }
    game_init();
    
    // Main Loop, o'rly?
    int render_start = 0, render_diff = 0, render_wait = 0;
    int fps_count = 0, fps_time = 0;
    while (engine->running) {
        render_start = SDL_GetTicks();
        
        engine_events();
        if (key_pressed(SDLK_p)) {
            engine_pause(!engine->paused);
        }
        if (!engine->paused) {
            game_update();
        }
        engine_clear_keys();
        engine_clear_mouse();
        game_render(engine->screen);
        SDL_Flip(engine->screen->bg);
        
        // Limit FPS
        render_diff = SDL_GetTicks() - render_start;
        if (render_diff > 0) {
            render_wait = engine->fps_wait - render_diff;
            if (render_wait > 0) {
                SDL_Delay(render_wait);
            }
            if (!engine->paused) {
                engine->time += SDL_GetTicks() - render_start;
            }
        }
        
        // Count FPS
        if (!engine->paused) {
            fps_count++;
            if (fps_count >= 10) {
                engine->fps_current = 1000 / ((engine->time - fps_time) / fps_count);
                fps_count = 0;
                fps_time = engine->time;
            }
        }
    }
    
    // Cleanup
    game_quit();
    engine_cleanup();
    return 0;
}

bool engine_init(int width, int height, int scale, int fps) {
    atexit(SDL_Quit);
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        engine_crash("SDL initilization failed");
    }
    
    // Init Screen
    engine->screen = screen_create(width, height, scale);
    fps_set(fps);
    
    // Stuff
    engine->mouse_x = 0;
    engine->mouse_y = 0;
    engine->list_tilemaps = list_create(8);
    engine->list_surfaces = list_create(16);
    engine->running = true;
    return true;
}

void engine_cleanup() {
    struct SDL_Surface *img;
    while ((img = (SDL_Surface*)list_pop(engine->list_surfaces, 0)) != NULL) {
        SDL_FreeSurface(img);
    }
    free(engine->list_surfaces);
    
    struct TileMap *tile_map;
    while ((tile_map = (struct TileMap*)list_pop(engine->list_tilemaps, 0)) != NULL) {
        free(tile_map);
    }
    free(engine->list_tilemaps);
    free(engine->screen);
    free(engine);
}

void engine_crash(const char *reason) {
    printf("%s: %s\n", reason, SDL_GetError());
    engine->running = false;
    engine_cleanup();
    exit(1);
}

void engine_pause(bool mode) {
    engine->paused = mode;
}

bool engine_paused() {
    return engine->paused;
}

void engine_quit() {
    engine->running = false;
}



// General Stuff ---------------------------------------------------------------
// -----------------------------------------------------------------------------
int fps_get(bool current) {
    return current ? engine->fps_current : engine->fps_rate;
}

void fps_set(int fps) {
    engine->fps_rate = fps;
    engine->fps_wait = 1000 / fps;
}

int time_get() {
    return engine->time;
}


// Events and Input ------------------------------------------------------------
// -----------------------------------------------------------------------------
void engine_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                engine->running = false;
                break;
            
            case SDL_KEYDOWN:
                engine->keys[event.key.keysym.sym] = 1;
                engine->key_count++;
                break;
            
            case SDL_KEYUP:
                engine->keys[event.key.keysym.sym] = 0;
                engine->key_count--;
                if (engine->key_count < 0) {
                    engine->key_count = 0;
                }
                break;
            
            case SDL_MOUSEMOTION:
                engine->mouse_x = event.motion.x;
                engine->mouse_y = event.motion.y;
                break;
            
            case SDL_MOUSEBUTTONDOWN:
                engine->mouse[event.button.button] = 1;
                engine->mouse_count++;
                break;
            
            case SDL_MOUSEBUTTONUP:
                engine->mouse[event.button.button] = 0;
                engine->mouse_count--;
                if (engine->key_count < 0) {
                    engine->mouse_count = 0;
                }
                break;
            
            case SDL_ACTIVEEVENT:
                switch (event.active.state) {                    
                    case SDL_APPINPUTFOCUS:
                        engine->screen->has_focus = event.active.gain == 1;
                        break;
                    
                    case SDL_APPMOUSEFOCUS:
                        engine->screen->has_mouse = event.active.gain == 1;
                        break;
                }
                break;
            
            default:
                break;
        }
    }
}

void engine_clear_keys() {
    for(int i = 0; i < 322; i++) {
        if (engine->keys[i] == 1) {
            engine->keys[i] = 2;
        }
    }
}

void engine_clear_mouse() {
    for(int i = 0; i < 8; i++) {
        if (engine->mouse[i] == 1) {
            engine->mouse[i] = 2;
        }
    }
}

bool key_down(const SDLKey key) {
    return engine->keys[key] > 0;
}

bool key_pressed(const SDLKey key) {
    return engine->keys[key] == 1;
}

int key_count() {
    return engine->key_count;
}

bool mouse_down(const int button) {
    return engine->mouse[button] > 0;
}

bool mouse_pressed(const int button) {
    return engine->mouse[button] == 1;
}

bool mouse_on_screen() {
    return engine->screen->has_mouse;
}

void mouse_get_pos(int *x, int *y) {
    *x = engine->mouse_x;
    *y = engine->mouse_y;
}


// Images ----------------------------------------------------------------------
// -----------------------------------------------------------------------------
SDL_Surface *image_create(int w, int h, long int key) {
    SDL_PixelFormat *format = engine->screen->format;
    SDL_Surface *img = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_RLEACCEL, w, h,
                                            format->BitsPerPixel, 
                                            format->Rmask,
                                            format->Gmask,
                                            format->Bmask,
                                            format->Amask);
    
    if (!img) {
        engine_crash("Surface creation failed");
    }
    image_color_key(img, key);
    list_add(engine->list_surfaces, img);
    return img;
}

SDL_Surface *image_load(const char* file, long int key) {
    SDL_Surface *tmp = IMG_Load(file);
    if (!tmp) {
        engine_crash("Image loading failed");
    }
    
    SDL_Surface *img = SDL_ConvertSurface(tmp, engine->screen->format, SDL_HWSURFACE | SDL_RLEACCEL);
    SDL_FreeSurface(tmp);
    image_color_key(img, key);
    list_add(engine->list_surfaces, img);
    return img;
}

void image_draw(SDL_Surface *bg, SDL_Surface *img, const int x, const int y) {
    SDL_Rect dst = {x, y, img->w, img->h};
    SDL_BlitSurface(img, NULL, bg, &dst);
}

void image_color_key(SDL_Surface *img, long int key) {
    if (key != -1 && SDL_SetColorKey(img, SDL_SRCCOLORKEY | SDL_RLEACCEL, key) == -1) {
        engine_crash("Colorkey setting failed");
    }
}


// Tiles -----------------------------------------------------------------------
// -----------------------------------------------------------------------------
struct TileMap *tiles_from_image(SDL_Surface *img, int w, int h) {    
    struct TileMap *tile_map = (struct TileMap*)malloc(sizeof(struct TileMap));
    tile_map->img = img;
    tile_map->w = w;
    tile_map->h = h;
    tile_map->xs = img->w / w;
    tile_map->ys = img->h / h;
    
    if (list_find(engine->list_tilemaps, tile_map) == -1) {
        list_append(engine->list_tilemaps, tile_map);
    }
    return tile_map;
}

struct TileMap *tiles_from_file(const char* file, int w, int h, long int key) {
    return tiles_from_image(image_load(file, key), w, h);
}

void tiles_draw(const struct TileMap *map, SDL_Surface *bg, const int index, int x, const int y) {
    const int ty = index / map->w;
    const int tx = index - ty * map->w;
    SDL_Rect src = {tx * map->xs, ty * map->ys, map->xs, map->ys};
    SDL_Rect dst = {x, y, map->xs, map->ys};
    SDL_BlitSurface(map->img, &src, bg, &dst); 
}


// Drawing ---------------------------------------------------------------------
// -----------------------------------------------------------------------------
int color_create(const int r, const int g, const int b) {
    return SDL_MapRGB(engine->screen->bg->format, r, g, b);
}
int color_create_alpha(const int r, const int g, const int b, const int a) {
    return SDL_MapRGBA(engine->screen->bg->format, r, g, b, a);
}

void draw_rect(SDL_Surface *bg, const int x, const int y, const int w, const int h, const int color) {
    SDL_Rect *clip = &bg->clip_rect;
    SDL_Rect rect;
    
    const int cx = clip->x;
    const int cw = cx + clip->w;
    const int cy = clip->y;
    const int ch = cy + clip->h;
    
    if (x <= cw && x + w >= cx) {
        int xs = x, xe = x + w;
        if (xs < cx) {
            xs = cx;
        }
        if (xe > cw) {
            xe = cw;
        }
        rect.x = xs;
        rect.w = xe - xs;
        rect.h = 1;
        if (y >= cy && y <= ch) {
            rect.y = y;
            SDL_FillRect(bg, &rect, color);
        }
        if (y + h - 1 <= ch && y + h - 1 >= 0) {
            rect.y = y + h - 1;
            SDL_FillRect(bg, &rect, color);
        }
    }
    
    if (y <= ch && y + h >= cy) {
        int ys = y, ye = y + h;
        if (ys < cy) {
            ys = cy;
        }
        if (ye > ch) {
            ye = ch;
        }
        rect.y = ys;
        rect.h = ye - ys;
        rect.w = 1;
        if (x >= cx && x <= cw) {
            rect.x = x;
            SDL_FillRect(bg, &rect, color);
        }
        if (x + w - 1 <= cw && x + w - 1 >= 0) {
            rect.x = x + w - 1;
            SDL_FillRect(bg, &rect, color);
        }
    }
}

void draw_rect_filled(SDL_Surface *bg, const int x, const int y, const int w, const int h, const int color) {
    SDL_Rect rect = {x, y, w, h};
    SDL_FillRect(bg, &rect, color);
}


// Screen ----------------------------------------------------------------------
// -----------------------------------------------------------------------------
struct Screen *screen_create(int width, int height, int scale) {
    struct Screen *screen = ((struct Screen*)malloc(sizeof(struct Screen)));
    screen->w = width;
    screen->h = height;
    screen->scale = scale;
    screen->has_focus = true;
    screen->bg = SDL_SetVideoMode(screen->w, screen->h, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_ANYFORMAT);
    screen->format = screen->bg->format;
    if (screen->bg == NULL) {
        char *msg = (char*)calloc(sizeof(char), 40);
        sprintf(msg, "Videomode %d%d failed",  screen->w, screen->h);
        engine_crash(msg);
        return NULL;
    }
    return screen;
}

struct Screen *screen_get() {
    return engine->screen;
}

