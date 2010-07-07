#include "macro.h"

struct Map;
struct Character;

struct Player {
    struct Character *chara;
    bool has_control;
    bool fall_off;
    bool jumped;
    bool crouch;
};

struct Player *player_create(struct Map *map, const int x, const int y);
void player_free(struct Player *player);

void player_control(struct Player *player);
void player_render(const struct Player *player);

