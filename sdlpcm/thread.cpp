#include <SDL.h>
#include<iostream>

static int log(void* args){
    if(nullptr == args)
        std::cout<<"null args"<<std::endl;

    for(int i=0;i<3;i++)
        std::cout << i;
    std::cout <<std::endl;
    return 0;
}

int main(){
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Thread* t = SDL_CreateThread(log,"thread",nullptr);
    std::cin.get();
    SDL_Quit();
}