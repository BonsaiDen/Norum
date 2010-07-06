#include "macro.h"
#include "game.h"

#ifdef EDITOR
#include "editor.h"
#endif

#include "loader.h"
#include "player.h"
#include "map.h"
#include "engine.h"

#include <time.h>
#include <stdlib.h>
#include <SDL.h>


bool gravity_mode;
bool playing_mode;
int bg_color;

struct TileMap *tiles;
struct Map *map;
struct Player *player;


// Methods
void gravity_set(bool mode) {
    gravity_mode = mode;
}
bool gravity_get() {
    return gravity_mode;
}

void playing_set(bool mode) {
    playing_mode = mode;
}

bool playing_get() {
    return playing_mode;   
}


// Game Functions --------------------------------------------------------------
// -----------------------------------------------------------------------------
void game_init() {
    time_t seconds;
    time(&seconds);
    srand((unsigned int)seconds);
    
    SDL_WM_SetCaption(WINDOW_TITLE, WINDOW_TITLE);
    
    // Stage
    gravity_set(true);
    playing_set(false);
    bg_color = color_create(64, 64, 64);
    
    tiles = tiles_from_file("data/tiles.png", 6, 2, color_create(255, 0, 255));
    map = map_create(WINDOW_WIDTH, WINDOW_HEIGHT, 200, 200, tiles);
    
    if (!map_load(map, "map.data")) {
        map_set_at(map, 0, 0, 1);
        map_set_at(map, 2, 2, 1);
        map_zone_create(map, 0, 0, 20, 15, 0, 0);
    }
    
    // Player
    player = player_create(map, 160, 120);
    map_set_player(map, player);
    
    #ifdef EDITOR
    editor_init(map);
    #else
    player->has_control = true;
    #endif
}

void game_update() {
    #ifdef EDITOR
    editor_mode(map, player);

    if (playing_mode) {
    #endif
        
        map_control_platforms_vertical(map);
        player_control(player);
        map_control_platforms_horizontal(map);
        map_control(map);
    
    #ifdef EDITOR
    } else {
        editor_update(map);
    }
    #endif
}

void game_render(struct Screen *screen) {
    SDL_FillRect(screen->bg, 0, bg_color);
    map_draw(map);
    player_render(player);
    map_render(map, screen->bg);
    
    #ifdef EDITOR
    if (!playing_mode) {
        editor_render(map, screen->bg);
    }
    #endif
}

void game_quit() {
    #ifdef EDITOR
    printf("saving map...\n");
    map_save(map, "map.data");
    printf("map saved...\n");
    #endif
    printf("freeing map...\n");
    map_free(map);
    printf("freeing player...\n");
    player_free(player);
}

