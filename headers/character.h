#include "macro.h"

struct Map;
struct Platform;

struct Character {
    struct Map *map;
    
    int x, y;
    int ox, oy;
    int cx, cy;
    int speed, move_speed, speed_relative;
    
    float grav;
    float grav_fall;
    float grav_add;
    float grav_max;
    float grav_min;
    float grav_zero;
    float grav_relative;
    
    bool on_ground;
    bool on_platform;
    bool use_platforms;
    struct Platform *platform;
    
    int w, h;
    int l, r;
};


struct Character *chara_create(struct Map *map, const int x, const int y,
                                                const int width, const int height);
                                                
void chara_free(struct Character *chara);

void chara_set_gravity(struct Character *chara, float add, float min, float max);
void chara_update(struct Character *chara, const bool has_control,
                                           const bool fall_off);

void chara_render(const struct Character *chara);

int chara_get_space_y(struct Character *chara);

