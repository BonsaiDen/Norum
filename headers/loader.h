#include "macro.h"

struct Map;


void map_save(struct Map *map, const char *file_name);
bool map_load(struct Map *map, const char *file_name);

