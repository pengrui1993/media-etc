#include <iostream>
#include <SDL.h>
#include <functional>
//音频块，音频的长度，以及当前播放位置
static const int frame_size = 4096;
static Uint8 * audioChunk;
static Uint32 audio_len;
static Uint8 *audio_pos;
static bool running{true};
static SDL_AudioSpec audioSpec; 
static FILE *fp;
static char* buffer;
#define TICK_NOW SDL_GetTicks

struct Scheduler{
private:
    using Tick = decltype(TICK_NOW());
    using Duration = int;
    using Runner = std::function<void(void)>;
    Runner runner;
    Tick pre;
    Duration dur;
    static auto now()->Tick const{return TICK_NOW();}
public:
    Scheduler(Runner fun):runner{fun},pre{now()},dur{1000}{}
    Scheduler(Runner fun,Duration d):runner{fun},pre{now()},dur{d}{}
    void trigger(){
        if(!runner)return;
        auto n =  now();
        if(n-pre<dur)return;
        runner();
        pre+=dur;
    }
};
struct Once{
    using Tick = decltype(TICK_NOW());
    using Runner = std::function<void(void)>;
private:
    Runner runner;
    bool executed{false};
    void trigger(){
        if(executed)return;
        executed = true;
        if(runner)runner();
    }
public:
    Once(Runner r):runner{r}{trigger();}
    
};
static bool ready_audio_read(){return audio_len != 0;}
static bool ready_to_write(){return audio_len==0;};
//回调函数
void fill_data_buffer(void *userData,Uint8 * stream,int len){
    static Scheduler printer{[](){std::cout << "callback:" << TICK_NOW() << std::endl;}};
    static Once once{[=](){std::cout<<"buf.len:"<<len<<std::endl;}};
    printer.trigger();
    SDL_memset(stream,0,len); //将stream置0
    if(!running)return;
    if(!ready_audio_read())return;
    len = len >audio_len? audio_len: len;
    SDL_MixAudio(stream,audio_pos,len,SDL_MIX_MAXVOLUME);  //混音处理
    audio_pos  += len;
    audio_len -= len;
}
static void loop(){
    int len;
    int count = 0;
    SDL_PauseAudio(0); //播放
    while(running){
	    if(count++>=100){
            std::cin.get();
            count = 0;
            running = false;
        }
        if (0==(len=fread(buffer,1,frame_size,fp)))fseek(fp,0,SEEK_SET);//读取数据
        if(!running)break;
        audio_len = len;
        audio_pos = (Uint8*)buffer;; //设置音频缓冲区的数据
        while(ready_audio_read())SDL_Delay(1);
    }
    SDL_CloseAudio();
}
int main(int argc,char** argv){
    if (SDL_Init(SDL_INIT_AUDIO)){
        std::cerr << "fail to initialize the SDL - " << SDL_GetError() << std::endl;
        return -1;
    }
    //ffplay -f s16le -ac 2 -ar 44100 theshow.pcm
    audioSpec.channels = 2;//设置audioSpec
    audioSpec.format = AUDIO_S16SYS;
    audioSpec.samples = 1024;
    audioSpec.silence = 0;
    audioSpec.freq = 44100;
    audioSpec.callback = fill_data_buffer;
    if (SDL_OpenAudio(&audioSpec,NULL) < 0){  //打开音频
        printf("fail to open audio\n");
        return -1;
    }
    fp = fopen(argv[1],"rb");
    if (fp == NULL){
        printf("fail to load image\n");
        return -1;
    }
    buffer = new char[frame_size];
    loop();
    SDL_Quit();
    delete[] buffer;
    fclose(fp);
    return 0;
}
