#include <stdbool.h>

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
    struct Platform *platform;
    
    int w, h;
    int l, r;
};


struct Character *chara_create(struct Map *map, const int x, const int y,
                                                const int width, const int height);

void chara_set_gravity(struct Character *chara, float add, float min, float max);
void chara_update(struct Character *chara, const bool has_control,
                                           const bool fall_off);

void chara_render(const struct Character *chara);

bool chara_move(struct Character *chara);
bool chara_check_x(struct Character *chara, const bool init, int offset);
void chara_limit_x(const struct Character *chara, int *mx, int *offset);
int chara_col_left(const struct Character *chara);
int chara_col_right(const struct Character *chara);

bool chara_fall(struct Character *chara, const bool fall_off);
bool chara_check_y(struct Character *chara, const bool init, const int offset);
void chara_limit_y(struct Character *chara, int *my, int *offset);
int chara_col_up(const struct Character *chara);
int chara_col_down(const struct Character *chara);

