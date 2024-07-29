#include <iostream>
#include <cstdlib>
#include <SDL.h>
#include <thread>
//音频块，音频的长度，以及当前播放位置
static Uint32 audio_len;
static Uint8 *audio_pos;
static SDL_AudioSpec audioSpec;
static const int frame_size = 4096;
static char *buffer;
static SDL_mutex* mutex;
static SDL_cond* cond;
static FILE *fp;
static int data_count = 0;
static SDL_Thread* thread{nullptr};
static bool thread_running {false};
static std::thread::id sound_thread_id;
static decltype(std::this_thread::get_id()) main_thread_id;
#define READY_TO_READ true
#define READY_TO_WRITE false
struct audio_buf{
    bool state;
    Uint8 buf[frame_size]{0};
    uint32_t consumed_len{0};
    uint32_t remain{0};
};
static const int num_buf = 2;
static int read_index{0};
static audio_buf bufs[num_buf];

static std::thread::id pre_tid;
static volatile bool show_callback_msg{true};
//回调函数
void fill_data_buffer(void *userData,Uint8 * stream,int len){
    SDL_memset(stream,0,len);//将stream置0
    audio_buf& b = bufs[read_index];
    len = len>b.remain?b.remain:len;
    SDL_MixAudio(stream,b.buf+b.consumed_len,len,SDL_MIX_MAXVOLUME);
    b.consumed_len+=len;
    b.remain-=len;
    if(0==b.remain){
        b.state = READY_TO_WRITE;
        SDL_CondBroadcast(cond);// SDL_CondSignal(cond);
    }
    auto temp_id = std::this_thread::get_id();
    if(temp_id!=pre_tid){
        /*
        callback    0x16d6ab000
        main        0x1dd5b1000
        sound       0x16d61f000
        */
       std::cout << "callback " << SDL_GetTicks()
            <<",callback.thread.id:" << std::this_thread::get_id() 
            <<",main.thread.id:"<<main_thread_id
            <<",sound.thread.id:"<<sound_thread_id
            << std::endl;
        pre_tid = temp_id;
    }
}

static int loop(void*){
    sound_thread_id = std::this_thread::get_id();
    if (SDL_OpenAudio(&audioSpec,NULL) < 0){
        std::cerr<<"fail to open audio\n" << std::endl;
        exit(-1);
    }
    SDL_PauseAudio(0);//播放
    thread_running = true;
    while(thread_running){
        SDL_LockMutex(mutex);
        for(int i=0;i<num_buf;i++){
            if(bufs[i].state == READY_TO_READ){
                SDL_Delay(1);
            }
        }
        read_index++;
        read_index%=num_buf;
        // SDL_CondSignal(cond);
        SDL_CondWait(cond,mutex);
        std::cout << "loop 4" << std::endl;
        SDL_UnlockMutex(mutex);
    }
    return 0;
}
static void start_thread(){
    if(nullptr!=thread){
        std::cout <<"thread created,ignore that call" << std::endl;
        return;
    }
    thread = SDL_CreateThread(loop,"audio",nullptr);
}
static void sdl_destroy(){
    //释放申请的资源
    SDL_DestroyCond(cond);
    SDL_DestroyMutex(mutex);
    SDL_Quit();
}
static void buff_destroy(){
    free(buffer);
    buffer = nullptr;
}
static void buff_init(){
    buffer = (char*)malloc(frame_size);
}
static void sdl_init(){
    if (SDL_Init(SDL_INIT_AUDIO)){
        std::cerr << "fail to initialize the SDL - " << SDL_GetError() << std::endl;
        exit(-1);
    }//ffplay -f s16le -ac 2 -ar 44100 theshow.pcm
    audioSpec.channels = 2;
    audioSpec.format = AUDIO_S16SYS;
    audioSpec.samples = 1024;
    audioSpec.silence = 0;
    audioSpec.freq = 44100;
    audioSpec.callback = fill_data_buffer;
    mutex = SDL_CreateMutex();
    cond = SDL_CreateCond();
}
void file_init(const char* filename){
    fp = fopen(filename,"rb");
    if (nullptr != fp) return;
    std::cerr << "fail to load image\n" << std::endl;
    exit(-1);
}
void file_destroy(){
    fclose(fp);
}
int main(int argc,char** argv){
    main_thread_id = std::this_thread::get_id();
    sdl_init();
    buff_init();
    file_init(argv[1]);
    start_thread();
    bool running = true;
    while(!thread_running)SDL_Delay(1);
    int count = 0;
    while(running){
        if(count++>300){
            show_callback_msg = false;
            std::cin.get();
            exit(0);
        }
        size_t read_from_file = fread(buffer,1,frame_size,fp);
        switch(read_from_file){
            case 0:{
                fseek(fp,0,SEEK_SET);
                data_count = 0;
            }break;
            default:{
                SDL_LockMutex(mutex);
                while(true){
                    auto& buf = bufs[(read_index+1)%num_buf];
                    if(buf.state==READY_TO_WRITE){
                        buf.consumed_len = 0;
                        buf.remain = read_from_file;
                        memcpy(buf.buf,buffer,read_from_file);
                        buf.state = READY_TO_READ;
                        // SDL_CondSignal(cond);
                        SDL_CondBroadcast(cond);
                        goto notify;
                    }else{
                        SDL_CondWait(cond,mutex);
                    }
                }
                notify:
                SDL_UnlockMutex(mutex);
            }
        }
    }
    std::cout << "exit"<< std::endl;
    sdl_destroy();
    buff_destroy();
    file_destroy();
    return 0;
}

class mut_test{
    mutable uint32_t acc_times{0};
public:
    void const_fun() const{
        acc_times++;
    }
};