#define EDITOR

#ifndef EDITOR
    #define WINDOW_WIDTH 320
    #define WINDOW_HEIGHT 240
    #define WINDOW_TITLE "Norum"

#else
    #define WINDOW_WIDTH 640
    #define WINDOW_HEIGHT 480
    #define WINDOW_TITLE "Norum Editor"
#endif


#ifdef WIN32
    #pragma comment(lib, "SDL.lib")
    #pragma comment(lib, "SDLmain.lib")
    #pragma comment(lib, "SDL_image.lib")
    
    #define fmin(x1,x2) ((x1) > (x2) ? (x2):(x1))
    #define fmax(x1,x2) ((x1) > (x2) ? (x1):(x2))
    
    #define bool char
    #define true 1
    #define false 0
    
#else
    #include <stdbool.h>
    #define likely(x)       __builtin_expect((x),1)
    #define unlikely(x)     __builtin_expect((x),0)
#endif
