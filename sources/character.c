#include "character.h"
#include "map.h"
#include "engine.h"

#include <stdlib.h>
#include <math.h>

void chara_col_down_platform(struct Character *chara);

struct Character *chara_create(struct Map *map, const int x, const int y,
                                                const int width, const int height) {
    
    struct Character *chara = malloc(sizeof(struct Character));
    chara->map = map;
    
    chara->x = x;
    chara->y = y;
    chara->ox = 0;
    chara->oy = 0;
    chara->cx = 0;
    chara->cy = 0;
    chara->w = width;
    chara->h = height;
    chara->l = 0 - width / 2;
    chara->r = width / 2;
    
    chara->speed = 0;
    chara->move_speed = 0;
    chara->speed_relative = 0;
    
    chara->grav = 0.0;
    chara->grav_fall = 0.0;
    chara->grav_add = 0.0;
    chara->grav_max = 0.0;
    chara->grav_min = 0.0;
    chara->grav_zero = 0.0;
    chara->grav_relative = 0.0;
    
    chara->on_ground = false;
    chara->on_platform = false;
    
    chara_set_gravity(chara, 1.55, -6, 6);
    return chara;
}


// Apply Movements and Gravity -------------------------------------------------
void chara_update(struct Character *chara, const bool has_control,
                                           const bool fall_off) {
    
    if (fall_off) {
        chara_fall(chara, fall_off);
    }
    
    if (has_control) {
        chara_move(chara);
        chara_fall(chara, fall_off);
        
        if (chara->on_ground) {
            chara->grav_fall = 0.0;
        
        } else {
            chara->grav_fall += chara->grav_add;
        }
        
        if (chara->grav_add > 0) {
            if (chara->grav_fall > chara->grav_max) {
                chara->grav_fall = chara->grav_max;
            }
        
        } else if (chara->grav_add < 0) {
            if (chara->grav_fall < chara->grav_min) {
                chara->grav_fall = chara->grav_min;
            }
        }
    }
}


// Horizontal Movement ---------------------------------------------------------
// -----------------------------------------------------------------------------
bool chara_move(struct Character *chara) {
    chara->grav_relative = chara->y - chara->oy;
    chara->oy = chara->y;
    
    int mx, offset;
    
    chara->speed = chara->move_speed;
    if (chara->on_platform) {
        if (chara->platform->mode == 2) {
            chara->speed += chara->platform->speed;
        
        } else if (chara->platform->mode == 8) {
            chara->speed -= chara->platform->speed;
        }
    }
    chara_limit_x(chara, &mx, &offset);
    chara_check_x(chara, true, 0);
    
    for (int i = 0, l = abs(chara->speed); i < l; i++) {
        if (chara_check_x(chara, false, offset)) {
            return true;
        }
        
        if (chara->speed < 0) {
            chara->x -= 1;
            if (chara->x < mx) {
                chara->x = mx;
                return true;
            }
            
        } else if (chara->speed > 0) {
            chara->x += 1;
            if (chara->x > mx) {
                chara->x = mx;
                return true;
            }
        }
    }
    return false;
}

bool chara_check_x(struct Character *chara, const bool init, const int offset) {
    int dy;
    if (chara->grav >= chara->grav_zero) {
        dy = chara_col_down(chara);
    
    } else {
        dy = chara_col_up(chara);
    }
    
    if (init) {
        chara->cy = dy;
    
    } else if (chara->grav_relative == 0 && chara->cy - dy != 0) {
        int ry = 0;
        if (chara->grav >= chara->grav_zero) {
            ry = map_col_down(chara->map, chara->x + offset, chara->y);
        
        } else {
            ry = map_col_up(chara->map, chara->x + offset, chara->y - chara->h);
        }
        
        if (ry == chara->cy) {
            return true;
        }
    }
    return false;
}

void chara_limit_x(const struct Character *chara, int *mx, int *offset) {
    if (chara->speed > 0) {
        *mx = chara_col_right(chara) - chara->r;
        *offset = chara->r;
    
    } else {
        *mx = chara_col_left(chara) - abs(chara->l);
        *offset = chara->l - 1; 
    }
}

int chara_col_left(const struct Character *chara) {
    int mx = 0, o = 0;
    for (int i = 0, l = chara->h + 15; i < l; i += 8) {
        o = i -1;
        if (o < 1) {
            o = 1;
        
        } else if (o > chara->h) {
            o = chara->h;
        }
        
        mx = fmax(map_col_left(chara->map, 
                              chara->x + chara->l,
                              chara->y - o), mx);
        
    }
    return mx;
}

int chara_col_right(const struct Character *chara) {
    int mx = chara->map->size_x * 16 + 16, o = 0;
    for (int i = 0, l = chara->h + 15; i < l; i += 8) {
        o = i -1;
        if (o < 1) {
            o = 1;
        
        } else if (o > chara->h) {
            o = chara->h;
        }
        
        mx = fmin(map_col_right(chara->map,
                                chara->x + chara->r,
                                chara->y - o), mx);
        
    }
    return mx;
}


// Vertical Movement -----------------------------------------------------------
// -----------------------------------------------------------------------------
bool chara_fall(struct Character *chara, const bool fall_off) {
    chara->speed_relative = chara->x - chara->ox;
    chara->ox = chara->x;
    
    if (fall_off) {
        chara->y += chara->grav_relative;
        return false;
    }
    
    chara->grav = chara->grav_fall;
    chara_col_down_platform(chara);
    if (chara->on_platform && chara->grav >= chara->grav_zero) {
        if (chara->platform->mode == 2 || chara->platform->mode == 8) {
            chara->grav = 0;
            chara->grav_fall = 0;
            chara->on_ground = true;
            chara->y = chara->platform->zone->y * 16 + chara->platform->y;
            return true;
        }
        if (chara->platform->mode == 1) {
            chara->on_ground = true;
            chara->y = chara->platform->zone->y * 16 + chara->platform->y;
            chara->grav_fall = 0;
            chara->grav = 0;
            return true;
        
        } else if(chara->platform->mode == 4) {
            chara->on_ground = true;
            chara->grav_fall = 0;
            chara->y = chara->platform->zone->y * 16 + chara->platform->y ;
            chara->grav = 0;
            return true;
        }
    }
    
    int my, offset;
    chara_limit_y(chara, &my, &offset);
    if (chara->on_ground) {
        return false;
    }

    int grav;
    if (chara->grav < 0) {
        grav = abs(fmax(floor(chara->grav), chara->grav_min));
    
    } else {
        grav = fmin(floor(chara->grav), chara->grav_max);
    }
    
    chara_check_y(chara, true, 0);
    for (int i = 0, l = fmax(grav, 1); i < l; i++) {
        if (chara_check_y(chara, false, offset)) {
            return true;
        }
        if (chara->grav < chara->grav_zero) {
            chara->y -= 1;
            if (chara->y < my) {
                chara->grav_fall = 0;
                chara->grav = 0;
                chara->y = my;
                if (chara->grav_add < 0) {
                    chara->on_ground = true;
                }
                return true;
            }
        
        } else if(chara->grav >= chara->grav_zero) {
            chara->y += 1;
            if (chara->y > my) {
                chara->grav_fall = 0;
                chara->grav = 0;
                chara->y = my;
                if (chara->grav_add > 0) {
                    chara->on_ground = true;
                }
                return true;
            }
        }
    }
    return false;
}

void chara_limit_y(struct Character *chara, int *my, int *offset) {
    if (chara->grav_add > 0) {
        if (chara->grav >= chara->grav_zero) {
            int y = chara_col_down(chara);
            chara->on_ground = y > chara->y ? false : true;
            *my = y;
            *offset = 0;
        
        } else {
            *my = chara_col_up(chara) + chara->h;
            *offset = chara->h + 1;
        }
    
    } else {
        if (chara->grav <= chara->grav_zero) {
            int y = chara_col_up(chara) + chara->h;
            chara->on_ground = y < chara->y ? false : true;
            *my = y;
            *offset = chara->h + 1;
        
        } else {
            *my = chara_col_down(chara);
            *offset = 0;
        }
    }
}

bool chara_check_y(struct Character *chara, bool init, int offset) {
    int dx;
    if (chara->speed < 0) {
        dx = chara_col_left(chara);
    
    } else {
        dx = chara_col_right(chara);
    }
    
    if (init) {
        chara->cx = dx;
    
    } else if (chara->speed_relative == 0 && chara->cx - dx != 0) {
        int rx = 0;
        if (chara->speed < 0) {
            rx = map_col_left(chara->map, chara->x, chara->y - offset);
        
        } else {
            rx = map_col_right(chara->map, chara->x, chara->y - offset);
        }
        
        if (rx == chara->cx) {
            return true;
        }
    }
    return false;
}

int chara_col_up(const struct Character *chara) {
    int my = 0, o = 0;
    for (int i = chara->l, l = chara->r + 15; i < l; i += 16) {
        o = i -1;
        if (o < chara->l) {
            o = chara->l;
        
        } else if (o > chara->r - 1) {
            o = chara->r - 1;
        }
        my = fmax(map_col_up(chara->map,
                             chara->x + o,
                             chara->y - chara->h), my);
    }
    return my;
}

int chara_col_down(const struct Character *chara) {
    int my = chara->map->size_y * 16 + 16, o = 0;
    for (int i = chara->l, l = chara->r + 15; i < l; i += 16) {
        o = i -1;
        if (o < chara->l) {
            o = chara->l;
        
        } else if (o > chara->r - 1) {
            o = chara->r - 1;
        }
        my = fmin(map_col_down(chara->map, chara->x + o, chara->y), my);
    }
    return my;
}

void chara_col_down_platform(struct Character *chara) {
    struct Platform *tmp;
    struct Platform *platform = NULL;
    chara->on_platform = false;
    int o = 0;
    for (int i = chara->l, l = chara->r + 15; i < l; i += 16) {
        o = i -1;
        if (o < chara->l) {
            o = chara->l;
        
        } else if (o > chara->r - 1) {
            o = chara->r - 1;
        }
        tmp = map_col_down_platform(chara->map, chara->x + o, chara->y + 2);
        if (tmp != NULL && (platform == NULL || tmp->zone->y * 16 + tmp->y < platform->zone->y * 16 + platform->y)) {
            platform = tmp;
            chara->platform = platform;
            chara->on_platform = true;
        }
    }
}

void chara_set_gravity(struct Character *chara, float add, float min, float max) {
    if (min != 0 && max != 0) {
        chara->grav_min = min;
        chara->grav_max = max;
    }
    chara->grav_add = add;
    chara->grav_zero = add > 0 ? 0 : 1;
}


// Drawing ---------------------------------------------------------------------
// -----------------------------------------------------------------------------
void chara_render(const struct Character *chara) {
    int x, y;
    map_to_buffer(chara->map, chara->x + chara->l, chara->y - chara->h, &x, &y);
    
    draw_rect_filled(chara->map->buffer, x, y, 
                                         chara->w, chara->h,
                                         color_create(255, 255, 0));
    
}

