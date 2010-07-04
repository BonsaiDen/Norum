#include "player.h"
#include "character.h"
#include "map.h"
#include "game.h"
#include "engine.h"

#include <stdlib.h>
#include <math.h>


struct Player *player_create(struct Map *map, const int x, const int y) {
    struct Player *player = ((struct Player*)malloc(sizeof(struct Player)));
    player->chara = chara_create(map, x, y, 16, 24);
    player->fall_off = false;
    player->has_control = false;
    player->jumped = false;
    return player;
}

void player_control(struct Player *player) {
    if (player->has_control) {
        if (key_down(SDLK_a)) {
            player->chara->move_speed = -3;
        
        } else if (key_down(SDLK_d)) {
            player->chara->move_speed = 3;
        
        } else {
            player->chara->move_speed = 0;
        }
        
        if (key_pressed(SDLK_SPACE) && !player->jumped 
            && player->chara->on_ground) {
            
            player->chara->on_ground = false;
            player->chara->grav_fall = gravity_get() ? -15.9 : 15.9;
            player->jumped = true;
        
        } else if (!key_down(SDLK_SPACE)) {
            if (player->jumped) {
                if (gravity_get()) {
                    player->chara->grav_fall = fmax(-1.15, player->chara->grav_fall);
                
                } else {
                    player->chara->grav_fall = fmin(1.15, player->chara->grav_fall);
                }
                player->jumped = false;
            }
        }
    }
    chara_update(player->chara, player->has_control, player->fall_off);
}

void player_render(const struct Player *player) {
    chara_render(player->chara);
}

