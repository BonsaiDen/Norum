#include "macro.h"

struct Screen;


void game_init();
void game_update();
void game_render(struct Screen *screen);
void game_quit();

void gravity_set(bool mode);
bool gravity_get();

void playing_set(bool mode);
bool playing_get();

