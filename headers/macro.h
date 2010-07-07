#ifndef EDITOR
    #define WINDOW_WIDTH 320
    #define WINDOW_HEIGHT 240
    #define WINDOW_TITLE "Norum"

#else
    #define WINDOW_WIDTH 640
    #define WINDOW_HEIGHT 480
    #define WINDOW_TITLE "Norum Editor"
#endif


// Some stuff that's not in C89
#define min(x1,x2) ((x1) > (x2) ? (x2):(x1))
#define max(x1,x2) ((x1) > (x2) ? (x1):(x2))

#define bool char
#define true 1
#define false 0

#ifdef WIN32
    #define likely(x)       (x)
    #define unlikely(x)     (x)
    
    // Visual Studio
    #ifdef _MSC_VER
        #pragma comment(lib, "SDL.lib")
        #pragma comment(lib, "SDLmain.lib")
        #pragma comment(lib, "SDL_image.lib")
    #endif

// GCC Linux
#else
    
    // Branching "optimization"
    #define likely(x)       __builtin_expect((x),1)
    #define unlikely(x)     __builtin_expect((x),0)
#endif

