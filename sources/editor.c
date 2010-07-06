#include "editor.h"
#include "list.h"
#include "player.h"
#include "character.h"
#include "map.h"
#include "game.h"
#include "engine.h"

#include <SDL_image.h>
#include <SDL.h>

#include <stdlib.h>
#include <stdio.h>


struct MapZone *editor_zone = NULL;
struct MapZone *editor_edit_zone = NULL;
int editor_old_zone[4];
int editor_move_zone_dir = 0;
int editor_move_offset_x = 0;
int editor_move_offset_y = 0;
int editor_pos_x = 0;
int editor_pos_y = 0;
int editor_tile_x = 0;
int editor_tile_y = 0;
bool editor_tiles = false;
int editor_draw_tile = 1;

int red;
int yellow;
int blue;

void editor_init(struct Map *map) {
    struct Screen *screen = screen_get();
    map_set_region(map, 0, 0, screen->w, screen->h);
    red = color_create(255, 0, 0);
    yellow = color_create(255, 255, 0);
    blue = color_create(0, 0, 255);
}

void editor_mode(struct Map *map, struct Player *player) {
    if (key_pressed(SDLK_RETURN)) {
        struct Screen *screen = screen_get();
        
        playing_set(!playing_get());
        if (playing_get()) {
            player->fall_off = false;
            player->has_control = true;
            player->chara->x = editor_tile_x * 16 + 8;
            player->chara->y = editor_tile_y * 16 + 16;
            
            int w = screen->w / 2 , h = screen->h / 2;
            map_set_region(map, w / 2, h / 2, w, h);
            map_platforms_create(map);
            
        } else {
            map_set_region(map, 0, 0, screen->w, screen->h);
            map_platforms_remove(map);
        }
        map->cur_zone = NULL;
        map->old_zone = NULL;
        map->scroll_to = 0;
    }
    
    if (key_pressed(SDLK_RCTRL)) {
        gravity_set(!gravity_get());
        player->chara->on_ground = false;
        chara_set_gravity(player->chara, gravity_get() ? 1.55 : -1.55, 0, 0);
        player->chara->grav /= 2.0;
    }
}

void editor_edit_zones(struct Map *map, int px, int py, bool shift, bool ctrl) {
    if (editor_zone != NULL && !shift) {
        int sx, sy, ex, ey;
        map_zone_get_region(editor_zone, &sx, &sy, &ex, &ey);
        
        // Delete
        if (mouse_pressed(SDL_BUTTON_RIGHT) && ctrl) {
            map_zone_delete(map, editor_zone);
            return;
        }
        
        // Resize
        int dxl = abs(sx * 16 - px), dxr = abs((sx + ex) * 16 - px);
        int dyu = abs(sy * 16 - py), dyd = abs((sy + ey) * 16 - py);
        if (editor_move_zone_dir == 0) {
            if (ctrl) {
                editor_tiles = false;
            }
        
            bool m = mouse_pressed(SDL_BUTTON_LEFT);
            if (ctrl && m) { 
                editor_move_zone_dir = 16;
                editor_move_offset_x = editor_tile_x - sx;
                editor_move_offset_y = editor_tile_y - sy;
           //     screen.setCursor('default');
            } else if (dxl <= 8) {
                editor_tiles = false;
              //  screen.setCursor('w-resize');
                if (m) {
                    editor_move_zone_dir = 8;
                }
            
            } else if (dxr <= 8) {
                editor_tiles = false;
         //       screen.setCursor('e-resize');
                if (m) {
                    editor_move_zone_dir = 2;
                }
            } else if (dyu <= 8 ) {
                editor_tiles = false;
             //   screen.setCursor('n-resize');
                if (m) {
                    editor_move_zone_dir = 1;
                }
            
            } else if (dyd <= 8) {
                editor_tiles = false;
               // screen.setCursor('s-resize');
                if (m) {
                    editor_move_zone_dir = 4;
                }
            }
            
            if (editor_move_zone_dir != 0) {
                editor_edit_zone = editor_zone;
                editor_old_zone[0] = sx;
                editor_old_zone[1] = sy;
                editor_old_zone[2] = ex;
                editor_old_zone[3] = ey;
            }
        }
        
        // Cancel Resize
        if (!mouse_down(SDL_BUTTON_LEFT)) {
            if (editor_move_zone_dir != 0) {
                editor_zone = NULL;
                editor_move_zone_dir = 0;
             //   pygame.mouse.set_cursor(*pygame.cursors.arrow)
            }
        }
        
        // Resize
        if (editor_move_zone_dir == 1) {
            int h = editor_old_zone[3] + (editor_old_zone[1] - editor_tile_y);
            if (h >= (editor_edit_zone->type == 0 ? 15 : 1)) {
                editor_edit_zone->y = editor_tile_y;
                editor_edit_zone->h = h;
            }
            
        } else if (editor_move_zone_dir == 8) {
            int w = editor_old_zone[2] + (editor_old_zone[0] - editor_tile_x);
            if (w >= (editor_edit_zone->type == 0 ? 20 : 2)) {
                editor_edit_zone->x = editor_tile_x;
                editor_edit_zone->w = w;
            }
        
        } else if (editor_move_zone_dir == 2) {
            int w = editor_old_zone[2] - ((editor_old_zone[2] - editor_tile_x) + editor_edit_zone->x) + 1;
            if (w >=( editor_edit_zone->type == 0 ? 20 : 2)) {
                editor_edit_zone->w = w;
            }
        
        } else if (editor_move_zone_dir == 4) {
            int h = editor_old_zone[3] - ((editor_old_zone[3] - editor_tile_y) + editor_edit_zone->y) + 1;
            if (h >= (editor_edit_zone->type == 0 ? 15 : 1)) {
                editor_edit_zone->h = h;
            }
        
        } else if(editor_move_zone_dir == 16) {
            int x = editor_tile_x - editor_move_offset_x;
            if (x < 0) {
                x = 0;
            }
            if (x > map->size_x - editor_old_zone[2]) {
                x = map->size_x - editor_old_zone[2];
            }
            editor_edit_zone->x = x;
            
            int y = editor_tile_y - editor_move_offset_y;
            if (y < 0) {
                y = 0;
            }
            if (y > map->size_y - editor_old_zone[3]) {
                y = map->size_y - editor_old_zone[3];
            }
            editor_edit_zone->y = y;
        }
        
        if (editor_move_zone_dir != 0) {
            editor_tiles = false;
        }
    }
}

void editor_update(struct Map *map) {
    // Camera Control
    if (key_down(SDLK_a)) {
        editor_pos_x -= 1;
    }
    
    if (key_down(SDLK_d)) {
        editor_pos_x += 1;
    }
    
    if (key_down(SDLK_w)) {
        editor_pos_y -= 1;
    }
    
    if (key_down(SDLK_s)) {
        editor_pos_y += 1;
    }
    
    map->pos_x = editor_pos_x * 16;
    map->pos_y = editor_pos_y * 16;
    map_offset(map, false, 0, 0);
    editor_pos_x = map->pos_x / 16;
    editor_pos_y = map->pos_y / 16;
    
    // Editing
    if (mouse_on_screen()) {
        int mx, my;
        mouse_get_pos(&mx, &my);
        
        int px, py;
        map_to_map(map, mx, my, &px, &py);
        editor_tile_x = px / 16;
        editor_tile_y = py / 16;
        editor_tiles = (editor_move_zone_dir == 0);
        
        // Zones
        editor_tiles = true;
        if (editor_move_zone_dir == 0) {
            struct MapZone *old_zone = editor_zone;
            editor_zone = map_zone_get_at(map, px, py, true);
            if (editor_zone == NULL && old_zone != NULL) {
           //     screen.setCursor('default');
                
            }
        }
        bool shift = key_down(SDLK_LSHIFT);
        bool ctrl = key_down(SDLK_LCTRL);
        editor_edit_zones(map, px, py, shift, ctrl);
        
        // Edit Tiles
        if (editor_tiles) {
            if (mouse_pressed(SDL_BUTTON_LEFT) || (shift && mouse_down(SDL_BUTTON_LEFT))) {
                map_set_at(map, editor_tile_x, editor_tile_y, editor_draw_tile);
            
            } else if (mouse_pressed(SDL_BUTTON_RIGHT) || (shift && mouse_down(SDL_BUTTON_RIGHT))) {
                map_set_at(map, editor_tile_x, editor_tile_y, 0);
            }
            
            if (key_pressed(SDLK_F1)) {
                map_zone_create(map, editor_tile_x, editor_tile_y, 20, 15, 0, 0);
            }
            
            if (key_pressed(SDLK_F2)) {
                map_zone_create(map, editor_tile_x, editor_tile_y, 9, 1, 1, 0);
            }  
            
            if (key_pressed(SDLK_1)) {
                editor_draw_tile = 1;
            
            } else if (key_pressed(SDLK_2)) {
                editor_draw_tile = 2;
            }
        }
        
        if (editor_zone) {
            if (editor_zone->h > 1) {
                if (key_pressed(SDLK_UP)) {
                    editor_zone->extra = 1;
                
                } else if (key_pressed(SDLK_DOWN)) {
                    editor_zone->extra = 4;
                }
            
            } else if (editor_zone->w > 2) {
                if (key_pressed(SDLK_LEFT)) {
                    editor_zone->extra = 8;
                
                } else if (key_pressed(SDLK_RIGHT)) {
                    editor_zone->extra = 2;
                }
            }
            if (key_pressed(SDLK_END)) {
                editor_zone->extra = 0;
            }
        }
    
    } else {
        editor_zone = NULL;
        editor_move_zone_dir = 0;
        editor_tiles = true;
    }
}

void editor_render(struct Map *map, SDL_Surface *bg) {
    // Cursor
    if (mouse_on_screen() && editor_tiles) {
        int tx, ty;
        map_to_screen(map, editor_tile_x * 16, editor_tile_y * 16, &tx, &ty);
        tiles_draw(map->tile_map, bg, 11, tx, ty);
    }
    
    // Tiles
    int px, py, w, h;
    for(int i = 0, l = map->zones->length; i < l; i++) {
        struct MapZone *zone = (struct MapZone*)list_get(map->zones, i);
        map_zone_get_region(zone, &px, &py, &w, &h);
        map_to_screen(map, px * 16, py * 16, &px, &py);
        draw_rect(bg, px, py, w * 16, h * 16, editor_zone == zone ? yellow : red);
    }
    
    for(int i = 0, l = map->platform_zones->length; i < l; i++) {
        struct MapZone *zone = (struct MapZone*)list_get(map->platform_zones, i);
        map_zone_get_region(zone, &px, &py, &w, &h);
        map_to_screen(map, px * 16, py * 16, &px, &py);
        draw_rect(bg, px, py, w * 16, h * 16, editor_zone == zone ? yellow : blue);
        
        if (zone->extra == 1) {
            draw_rect_filled(bg, px + 2, py + 2, 28, 4, editor_zone == zone ? yellow : blue);  
        
        } else if (zone->extra == 4) {
            draw_rect_filled(bg, px + 2, py + h * 16 - 6, 28, 4, editor_zone == zone ? yellow : blue);  
        
        } else if (zone->extra == 0) {
            draw_rect_filled(bg, px + w * 16 / 2 - 14, py + h * 16 / 2 - 6, 28, 4, editor_zone == zone ? yellow : blue);
        
        } else if (zone->extra == 8) {
            draw_rect_filled(bg, px + 2, py + 2, 28, 4, editor_zone == zone ? yellow : blue);  
        
        } else if (zone->extra == 2) {
            draw_rect_filled(bg, px + w * 16 - 28, py + 2, 28, 4, editor_zone == zone ? yellow : blue);  
        }
    }
}

