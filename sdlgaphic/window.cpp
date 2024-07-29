#include<SDL.h>
#include <iostream>
static SDL_Window* sdlWindow;
static SDL_Renderer* sdlRenderer;
static SDL_Texture* sdlTexture;
static SDL_Surface* surface;
int main(){
    static const int width  = 200,height = 200;
    static const int rmask = 0x00FF0000,gmask = 0x0000FF00,bmask = 0x000000FF;
    static const int amask = 0xFF000000;//SDL_INIT_EVERYTHING
    if(0!=SDL_Init(SDL_INIT_EVENTS|SDL_INIT_VIDEO))return -1;
    if(0!=SDL_CreateWindowAndRenderer(width,height,0,&sdlWindow,&sdlRenderer))return -2;
    surface = SDL_CreateRGBSurface(0,200,height,32,rmask,gmask,bmask,amask);
    if(nullptr == surface) return -3;
    sdlTexture = SDL_CreateTexture(sdlRenderer
        ,SDL_PIXELFORMAT_ARGB8888
        ,SDL_TEXTUREACCESS_STREAMING
        ,width,height
        );
    int x, y;
    SDL_GetWindowPosition(sdlWindow, &x, &y);//SDL_SetWindowPosition
    std::printf("x:%d,y:%d\n",x,y);
    SDL_Rect rc;
    rc.x = rc.y = 0;
    rc.w = 10;
    rc.h = 10;
    int index = 0;
    for(int line = 0;line<height;line+=rc.h){
        for(int pos = 0;pos<width;pos+=rc.w){
            rc.x = pos;
            rc.y = line;
            const auto color = ((index++)%2==0)?0xff0000ff:0xffff0000;
            SDL_FillRect(surface,&rc,color);
        }
        if(line%2==0)index++;
    }
    SDL_UpdateTexture(sdlTexture, NULL, surface->pixels, surface->pitch);
    SDL_RenderClear(sdlRenderer);
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);
    SDL_Event evt;
    bool running = true;
    while(running){
        while(SDL_PollEvent(&evt)>0){
            switch(evt.type){
                case SDL_QUIT:{
                    std::printf("quit:%d\n",evt.type);
                    running = false;
                }break;
                case SDL_WINDOWEVENT:{
                    switch(evt.window.event){
                        case SDL_WINDOWEVENT_CLOSE:
                        std::printf("window:%d\n",evt.type);
                        break;
                    }
                }
            }
        }
        SDL_Delay(10);
    }
    std::printf("out loop\n");
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(sdlTexture);
    SDL_DestroyWindow(sdlWindow);
    SDL_DestroyRenderer(sdlRenderer);
    // SDL_DestroyWindowSurface(sdlWindow);
    SDL_Quit();
    return 0;
}