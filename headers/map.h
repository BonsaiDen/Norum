#include <SDL.h>
#include <stdbool.h>

struct List;
struct TileMap;
struct Player;

struct MapZone {
    int x, y, w, h;
    int type;
};

struct Platform {
    int x, y;
    int speed;
    int mode;
    struct MapZone *zone;
};

struct Map {
    struct TileMap *tile_map;
    SDL_Surface *buffer;
    
    unsigned char *blocks;
    
    int size_x, size_y;
    int tiles_x, tiles_y;
    
    int map_x, map_y;
    int scroll_x, scroll_y;
    int screen_x, screen_y;
    
    int pos_x, pos_y;
    int draw_x, draw_y;
    
    int scroll_offset;
    int scroll_to;
    
    struct Player *player;
    struct List *zones;
    struct MapZone *cur_zone;
    struct MapZone *old_zone;
    bool zone_fall;
    
    struct List *platform_zones;
    struct List *platforms;
    struct List *platforms_local;
};


// General
struct Map *map_create(const int width, const int height,
                        const int size_x, const int size_y,
                        struct TileMap *tiles);

void map_free(const struct Map *map);

void map_set_player(struct Map *map, struct Player *player);

void map_set_region(struct Map *map, const int x, const int y,
                                     const int w, const int h);

void map_draw(const struct Map *map);
void map_render(const struct Map *map, SDL_Surface *bg);

unsigned char map_get_at(const struct Map *map, const int x, const int y);
void map_set_at(struct Map *map, const int x, const int y, const int block);

void map_zone_create(const struct Map *map, const int x, const int y,
                                            const int w, const int h,
                                            const int type);

struct MapZone *map_zone_get_at(const struct Map *map, const int x,
                                                       const int y,
                                                       const bool platform_zones);

void map_zone_get_region(const struct MapZone *zone, int *x, int *y, int *w, int *h);
void map_zone_delete(const struct Map *map, struct MapZone *zone);
void map_zones_remove(const struct Map *map);


void map_platforms_create(const struct Map *map);
void map_platforms_remove(const struct Map *map);

// Camera
void map_to_map(const struct Map *map, const int x, const int y,
                                       int *px, int *py);

void map_to_screen(const struct Map *map, const int x, const int y,
                                          int *px, int *py);

void map_to_buffer(const struct Map *map, const int x, const int y,
                                          int *px, int *py);

bool map_offset(struct Map *map, const bool limit, const int zx, const int zy);
void map_control_platforms_vertical(struct Map *map);
void map_control_platforms_horizontal(struct Map *map);
void map_control(struct Map *map);

// Collision
int map_col_down(const struct Map *map, const int x, const int y);
int map_col_up(const struct Map *map, const int x, const int y);
int map_col_right(const struct Map *map, const int x, const int y);
int map_col_left(const struct Map *map, const int x, const int y);

struct Platform *map_col_down_platform(const struct Map *map, const int x, const int y);

