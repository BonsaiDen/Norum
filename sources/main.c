#include "macro.h"
#include "engine.h"


int main(int argc, char *argv[]) {
    int status = engine_create(WINDOW_WIDTH, WINDOW_HEIGHT, 1, 30);
    return status;
}

