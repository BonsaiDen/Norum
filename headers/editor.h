#include <SDL.h>
#include "macro.h"

struct Player;
struct Map;


void editor_init(struct Map *map);
void editor_mode(struct Map *map, struct Player *player);
void editor_update(struct Map *map);
void editor_render(struct Map *map, SDL_Surface *bg);

