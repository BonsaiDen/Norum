#include "main.h"
#include "engine.h"


int main(void) {
    int status = engine_create(WINDOW_WIDTH, WINDOW_HEIGHT, 1, 30);
    return status;
}

