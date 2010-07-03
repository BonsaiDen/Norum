#include "map.h"
#include "player.h"
#include "character.h"
#include "game.h"
#include "engine.h"
#include "list.h"

#include <stdlib.h>
#include <SDL.h>


struct Map *map_create(const int width, const int height,
                       const int size_x, const int size_y,
                       struct TileMap *tile_map) {
    
    struct Map *map = (struct Map*)malloc(sizeof(struct Map));
    
    map->blocks = (unsigned char*)calloc(sizeof(unsigned char), size_x * size_y);
    map->size_x = size_x;
    map->size_y = size_y;
    map->tiles_x = 0;
    map->tiles_y = 0;
    map->scroll_x = 0;
    map->scroll_y = 0;
    map->pos_x = 0;
    map->pos_y = 0;  
    map->map_x = 0;
    map->map_y = 0;
    
    map->tile_map = tile_map;
    map->buffer = NULL;
    map_set_region(map, 0, 0, width, height);
    
    map->scroll_offset = 0;
    map->scroll_to = 0;
    
    map->zones = list_create(4);
    map->cur_zone = NULL;
    map->player = NULL;
    
    map_offset(map, false, 0, 0);
    return map;
}

void map_set_player(struct Map *map, struct Player *player) {
    map->player = player;
}


// Drawing ---------------------------------------------------------------------
// -----------------------------------------------------------------------------
void map_draw(const struct Map *map) {
    int ox = map->scroll_x != 0 ? 1 : 0;
    int oy = map->scroll_y != 0 ? 1 : 0;
    
    SDL_FillRect(map->buffer, 0, color_create(0, 0, 0));
    int block, sx = 0, sy = 0;
    for (int y = map->map_y, my = map->map_y + map->tiles_y + oy; y < my; y++) {
        for (int x = map->map_x, mx = map->map_x + map->tiles_x + ox; x < mx; x++) {
            block = map_get_at(map, x, y);
            if (block != 0) {
                tiles_draw(map->tile_map, map->buffer, block - 1,
                           sx - map->scroll_x, sy - map->scroll_y);
                
            }
            sx += 16;
        }
        sx = 0;
        sy += 16;
    }
}

void map_render(const struct Map *map, SDL_Surface *bg) {
    image_draw(bg, map->buffer, map->draw_x, map->draw_y);
}

unsigned char map_get_at(const struct Map *map, const int x, const int y) {
    if (x < 0 || y < 0 || x > map->size_x - 1 || y > map->size_y - 1) {
        return 1;
    
    } else {
        return map->blocks[y * map->size_x + x];
    }
}

void map_set_at(struct Map *map, const int x, const int y, const int block) {
    if (x >= 0 && y >= 0 && x < map->size_x && y < map->size_y) {
        map->blocks[y * map->size_x + x] = block;
    }
}

void map_set_region(struct Map *map, const int x, const int y,
                                     const int w, const int h) {
    
    if (map->buffer != NULL) {
        SDL_FreeSurface(map->buffer);
    }
    map->buffer = image_create(w, h, -1);
    map->draw_x = x;
    map->draw_y = y;
    map->tiles_x = w / 16;
    map->tiles_y = h / 16;    
}


// Zones -----------------------------------------------------------------------
// -----------------------------------------------------------------------------
struct MapZone *map_zone_get_at(const struct Map *map, const int x,
                                                       const int y) {
    
    int px, py, w, h;
    struct MapZone *zone;
    for(int i = 0, l = map->zones->length; i < l; i++) {
        zone = (struct MapZone*)list_get(map->zones, i);
        map_zone_get_region(zone, &px, &py, &w, &h);
        if (x >= px * 16 && x < px * 16 + w * 16 
            && y >= py * 16 && y < py * 16 + h * 16) {
            
            return zone;
        }  
    }
    return NULL;
}

void map_zone_get_region(const struct MapZone *zone, int *x, int *y,
                                                     int *w, int *h) {
    
    *x = zone->x;
    *y = zone->y;
    *w = zone->w;
    *h = zone->h;
}

void map_zone_create(const struct Map *map, const int x, const int y,
                                            const int w, const int h) {
    
    struct MapZone *zone = (struct MapZone*)malloc(sizeof(struct MapZone));
    if (x + w > map->size_x) {
        zone->x = x - ((x + w) - map->size_x);
    } else {
        zone->x = x;
    }
    
    if (y + h > map->size_y) {
        zone->y = y - ((y + h) - map->size_y);
    } else {
        zone->y = y;
    }
    
    zone->w = w;
    zone->h = h;
    list_append(map->zones, zone);
}

void map_zone_delete(const struct Map *map, struct MapZone *zone) {
    free((struct MapZone*)list_remove(map->zones, zone));
}

void map_zones_remove(const struct Map *map) {
    struct MapZone *zone;
    while((zone = (struct MapZone*)list_pop(map->zones, 0)) != NULL) {
        free(zone);
    }
}

void map_zone_fall(const struct Map *map) {
    map->player->has_control = false;
    map->player->fall_off = true;
}


// Camera and Zones ------------------------------------------------------------
// -----------------------------------------------------------------------------
void map_to_screen(const struct Map *map, const int x, const int y,
                                          int *px, int *py) {
    
    *px = x - map->screen_x + map->draw_x;
    *py = y - map->screen_y + map->draw_y;
}

void map_to_map(const struct Map *map, const int x, const int y,
                                       int *px, int *py) {
    
    *px = x + map->screen_x - map->draw_x;
    *py = y + map->screen_y - map->draw_y;
}

void map_to_buffer(const struct Map *map, const int x, const int y,
                                          int *px, int *py) {
    
    *px = x - map->screen_x;
    *py = y - map->screen_y;
}

bool map_zone_change(struct Map *map, const struct MapZone *old, 
                                      const struct MapZone *next,
                                      const int x, const int y) {
    
    if (old == NULL) {
        return false;
    }
    
    int osx, osy, oex, oey;
    map_zone_get_region(old, &osx, &osy, &oex, &oey);
    if (next == NULL) {
        if (y == (osy + oey) * 16) {
            map_zone_fall(map);
        
        } else if (y > (osy + oey) * 16) {
            
            map_zone_fall(map);
        
        } else if (y < osy * 16) {
            map_zone_fall(map);
        }
        return false;
    }
    
    int zsx, zsy, zex, zey;
    map_zone_get_region(next, &zsx, &zsy, &zex, &zey);
    if (map->player->chara->grav_add > 0) {
        if (map_col_down(map, x, y) == zsy * 16) {
            map_zone_fall(map);
            return false;
        }
    
    } else if (map_col_up(map, x, y) == (zsy + zey) * 16) {
        map_zone_fall(map);
        return false;
    }
    
    // Down
    if (zsx + zex == osx + oex || zsx == osx) {
        int dl = abs(x - zsx * 16);
        int dr = abs(x - (zsx + zex) * 16);
        map->map_x = dr < dl ? (zsx + zex) - map->tiles_x : zsx;
        map->screen_x = map->pos_x = map->map_x * 16;
        map->scroll_x = 0;
        
        if (zsy > osy) {
            map->map_y = zsy - map->tiles_y;
            map->screen_y = map->pos_y = map->map_y * 16;
            map->scroll_y = 0;
            map->scroll_to = 4;
            map->player->has_control = false;
            map->scroll_offset = map->tiles_y * 16;
            return true;
            
        } else if (zsy < osy) {
            map->scroll_to = 1;
            map->map_y = osy;
            map->screen_y = map->pos_y = map->map_y * 16;
            map->scroll_y = 0;
            map->player->has_control = false;
            map->scroll_offset = map->tiles_y * 16;
            return true;
        }
    }
    
    // Sides
    if (zsy + zey == osy + oey || zsy == osy) {
        int du = abs(y - zsy * 16);
        int dd = abs(y - (zsy + zey) * 16);
        map->map_y = dd < du ? (zsy + zey) - map->tiles_y : zsy;
        map->screen_y = map->pos_y = map->map_y * 16;
        map->scroll_y = 0;
        
        // Right
        if (zsx > osx) {
            map->scroll_to = 2;
            map->map_x = zsx - map->tiles_x;
            map->screen_x = map->pos_x = map->map_x * 16;
            map->scroll_x = 0;
            map->player->chara->speed = 0;
            map->player->has_control = false;
            map->player->chara->x = (map->map_x + map->tiles_x) * 16  + 1;
            map->scroll_offset = map->tiles_x * 16;
            return true;
        
        // Left
        } else if (zsx < osx) {
            map->scroll_to = 8;
            map->map_x = osx;
            map->screen_x = map->pos_x = map->map_x * 16;
            map->scroll_x = 0;
            map->player->chara->speed = 0;
            map->player->has_control = false;
            map->player->chara->x = map->map_x * 16 - 1;
            map->scroll_offset = map->tiles_x * 16;
            return true;
        }
    }
    return false;
}

bool map_limit_to_zone(struct Map *map, const int x, const int y) {
    struct MapZone *zone = map_zone_get_at(map, x, y);
    if (zone != map->cur_zone && map->cur_zone != NULL
        && !map->player->fall_off) {
        
        if (map_zone_change(map, map->cur_zone, zone, x, y)) {
            map->cur_zone = zone;
            return true;
        }
    }
    
    if (map->cur_zone == NULL) {
        map->cur_zone = zone;
    }
    
    if (map->cur_zone == NULL) {
        return false;
    }
    
    int sx, sy, ex, ey;
    map_zone_get_region(map->cur_zone, &sx, &sy, &ex, &ey);
    if (map->map_x < sx) {
        map->map_x = sx;
        map->scroll_x = 0;
    }
    
    if (map->map_x >= sx + ex - map->tiles_x) {
        map->map_x = sx + ex - map->tiles_x;
        map->scroll_x = 0;
    }
    
    if (map->map_y < sy) {
        map->map_y = sy;
        map->scroll_y = 0;
    }
    
    if (map->map_y >= sy + ey - map->tiles_y) {
        map->map_y = sy + ey - map->tiles_y;
        map->scroll_y = 0;
    }
    return false;
}

bool map_offset(struct Map *map, const bool limit, const int zx, const int zy) {
    if (map->pos_x < 0) {
        map->pos_x = 0;
    
    } else if (map->pos_x > (map->size_x - map->tiles_x) * 16) {
        map->pos_x = (map->size_x - map->tiles_x) * 16;
    }
    
    if (map->pos_y < 0) {
        map->pos_y = 0;
    
    } else if (map->pos_y > (map->size_y - map->tiles_y) * 16) {
        map->pos_y = (map->size_y - map->tiles_y) * 16;
    }
    
    map->map_x = map->pos_x / 16;
    map->map_y = map->pos_y / 16;
    if (map->map_x < 0) {
        map->map_x = 0;
    }
    if (map->map_y < 0) {
        map->map_y = 0;
    }

    if (map->map_x >= map->size_x - map->tiles_x) {
        map->map_x = map->size_x - map->tiles_x;
    }
    
    if (map->map_y >= map->size_y - map->tiles_y) {
        map->map_y = map->size_y - map->tiles_y;
    }
    
    map->scroll_x = map->pos_x - map->map_x * 16;
    map->scroll_y = map->pos_y - map->map_y * 16;
    
    if (limit && map_limit_to_zone(map, zx, zy)) {
        return true;
    }
    
    if (map->scroll_x < 0) {
        map->scroll_x = 0;
    }
    
    if (map->scroll_y < 0) {
        map->scroll_y = 0;
    }
    
    map->screen_x = map->map_x * 16 + map->scroll_x; 
    map->screen_y = map->map_y * 16 + map->scroll_y;
    return false;
}


// Scroll Handling -------------------------------------------------------------
// -----------------------------------------------------------------------------
void map_control(struct Map *map) {
    struct Player *player = map->player;
    
    if (map->scroll_to != 0) {
        player->has_control = false;
        map->scroll_offset -= 14;
        
        if (map->scroll_to == 2) {
            map->pos_x += 14;
            if (map->scroll_offset <= 0) {
                map->pos_x += map->scroll_offset;
                player->has_control = true;
                map->scroll_to = 0;
            }
            player->chara->x += 1;
        
        } else if (map->scroll_to == 8) {
            map->pos_x -= 14;
            if (map->scroll_offset <= 0) {
                map->pos_x -= map->scroll_offset;
                player->has_control = true;
                
                map->scroll_to = 0;
            }
            player->chara->x -= 1;   
        
        } else if (map->scroll_to == 4) {
            map->pos_y += 14;
            if (map->pos_y % 6) {
                player->chara->y += 1;
            }
            if (map->scroll_offset <= 0) {
                map->pos_y += map->scroll_offset;
                player->has_control = true;
                map->scroll_to = 0;
            }
        
        } else if (map->scroll_to == 1) {
            map->pos_y -= 14;
            if (map->pos_y % 6) {
                player->chara->y -= 1;
            }
            if (map->scroll_offset <= 0) {
                map->pos_y -= map->scroll_offset;
                player->has_control = true;
                map->scroll_to = 0;
            }
        }
        map_offset(map, false, 0, 0);
    
    } else {
        map->pos_x = player->chara->x - (map->tiles_x / 2) * 16;
        map->pos_y = player->chara->y - (map->tiles_y / 2) * 16;
        
        int oy;
        if (player->chara->grav_add > 0) {
            oy = player->chara->y;
        
        } else {
            oy = player->chara->y - player->chara->h - 1;
        }
        map_offset(map, true, player->chara->x, oy);
    }
}


// Collision -------------------------------------------------------------------
// -----------------------------------------------------------------------------
int map_col_vertical(const struct Map *map, const int x,
                                            const int y,
                                            const int add) {
    
    const int px = x / 16;
    int py = y / 16;
    unsigned char block;
    int dec = 16;
    for (;;) {
        block = map_get_at(map, px, py);
        if (block == 1) {
            break;
        
        } else if (block == 2) {
            dec = 8;
            break;
        }
        py += add;
    }
    return py * 16 + (add == 1 ? 0 : dec);
}

int map_col_down(const struct Map *map, const int x, const int y) {
    return map_col_vertical(map, x, y, 1);
}

int map_col_up(const struct Map *map, const int x, const int y) {
    return map_col_vertical(map, x, y, -1);
}

int map_col_horizontal(const struct Map *map, const int x,
                                              const int y,
                                              const int add) {
    
    int px = x / 16;
    const int py = y / 16;
    const int ry = y - py * 16;
    unsigned char block;
    for (;;) {
        block = map_get_at(map, px, py);
        if (block == 1) {
            break;
        
        } else if (block == 2 && ry < 8) {
            break;
        }
        px += add;
    }
    return px * 16 + (add == 1 ? 0 : 32);
}

int map_col_right(const struct Map *map, const int x, const int y) {
    return map_col_horizontal(map, x, y, 1);
}

int map_col_left(const struct Map *map, const int x, const int y) {
    return map_col_horizontal(map, x, y, -1);
}

