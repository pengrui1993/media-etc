#include <iostream>
#include <vector>
#include <unordered_map>
#include <thread>
#include <cmath>
#include <SDL.h>
#define HARF_SQ(n) powf(2,n/12.0)
#define TEST_BUF_SIZE 2048
#define MIN(a,b) ((a>b)?b:a)
#define SAMPLE_FREQ (44100/2)
#define PI 3.14159
using u8 = Uint8;
using u64 = uint64_t;
static bool running = true;
static Uint32 audio_len;
static Uint8 *audio_pos;
static u64 tick = 0;
static u8 dur = 10;

static const int C_MAP[] = {0,2,4,5,7,9,11,12};
static const double C4 = 220*pow(2,3.0/12.0);
static const double A4 = 440;
static const int A_MAP [] ={0,2,3,5,7,8,10,12};
#define FREQ_FUN(n) [n] = A4*pow(2,C_MAP[n]/12.0)
static const double C_FREQ_MAP[]{
    FREQ_FUN(0)
    ,FREQ_FUN(1)
    ,FREQ_FUN(2)
    ,FREQ_FUN(3)
    ,FREQ_FUN(4)
    ,FREQ_FUN(5)
    ,FREQ_FUN(6)
    ,FREQ_FUN(7)
};
static int pre_pitch = 6;
static int post_pitch = 6;
static double start_freq = A4;
static double end_freq = A4;
static double process = 1;

static std::thread* key_thread;
static int _8bit_max = (1<<8)-1;
static int play_freq_index = 0;
// static double omiga = 2*PI*freq; 

static u8 noise0(){
    static const double init_freq = 440;
    static double freq = init_freq;
    // double val = 0.5*sin(2*PI *freq*  (tick++)*(1.0/SAMPLE_FREQ))+0.5;// [0,1]
    double val = 0.5*sin(2*PI *C_FREQ_MAP[play_freq_index]
        * (tick++)*(1.0/SAMPLE_FREQ))+0.5;// [0,1]
    tick%=SAMPLE_FREQ;// that line wile create crab noise
    return (u8)(_8bit_max*val);
}
static u8 noise(){
    tick++;
    double start = sin(2*PI *start_freq*  (tick)*(1.0/SAMPLE_FREQ));// [-1,1]
    double end   = sin(2*PI *end_freq*  (tick)*(1.0/SAMPLE_FREQ));// [-1,1]
    double mixer = (1-process)*start + process*end;//[-2,2]
    double val = mixer*0.25+0.5;//[0-1]
    // tick%=SAMPLE_FREQ;
    const u8 sound = (u8)(_8bit_max*val);;
    return sound;
}
void fill_data_buffer(void *userData,Uint8 * stream,int len){
    static u8 buf[TEST_BUF_SIZE];
    static Uint8* p;
    SDL_memset(stream,0,len);
    if(0==len)return;
    int buf_len = TEST_BUF_SIZE;
    p = buf;
    len = MIN(buf_len,len);
    for(int i=0;i<len;i++)p[i]=noise();
    SDL_MixAudio(stream,p,len,SDL_MIX_MAXVOLUME);
}

static void change_freq0(int start,int end){
    if(start == end)return;
    start_freq = C_FREQ_MAP[start-1];
    end_freq =C_FREQ_MAP[end-1];
}
static bool change_step = false;
static void change_freq(int start,int end){
    if(start == end)return;
    
    double target_start_wave_freq = C_FREQ_MAP[start-1];
    double target_end_wave_freq = C_FREQ_MAP[end-1];
    std::printf("start:%d,end :%d\n",start,end);
    std::printf("fstart:%lf,fend :%lf\n",target_start_wave_freq,target_end_wave_freq);
    if(!change_step){
        start_freq = target_start_wave_freq;
        end_freq = target_end_wave_freq;
        return;
    }
  
    static const double step = 0.1;
    double temp_start = start_freq;
    double temp_end = end_freq;
    auto temp = process;
    static const int delay = 3;
    while(temp>0){
        process = temp;
        temp-=step;
        start_freq = temp_start;
        //step go to start_wave_freq
        temp_start += (target_start_wave_freq-start_freq)*step;
        end_freq = temp_end;
        //step go to end_wave_freq
        temp_end += (target_end_wave_freq - end_freq)*step;
        // SDL_Delay(delay);
    }
    start_freq = target_start_wave_freq;
    end_freq = target_end_wave_freq;
    process = temp = 0;
    while(temp<1){
        // SDL_Delay(delay);
        process = temp;
        temp+=step;
    }
    process = 1;
}
static void key(){
    int cmd;
    while(running){
        std::printf("q for quit ,1-8 select pitch\n");
        ignore:
        switch(cmd=std::cin.get()){
            case 'q':running = false;continue;
            case '\n':goto ignore;// ignore newline
            case '+':play_freq_index++;play_freq_index%=8;break;
            default:
            if(cmd >='1'&&cmd<='8'){
                auto target = cmd-'0';
                change_freq(pre_pitch,target);
                pre_pitch = post_pitch;
                post_pitch = target;
                break;
            }
            std::cout <<"q for quit,continue."
                <<"current cmd:"<<(char)cmd
                <<",num:"<<cmd<< std::endl;
            break;
        }
    }
    audio_len = 0;
    std::cout << "thread quit" << std::endl;
}
int main(int argc,char** argv){
    if (SDL_Init(SDL_INIT_AUDIO)){
        std::cerr << "fail to initialize the SDL - " << SDL_GetError() << std::endl;
        return -1;
    }
    //ffplay -f s16le -ac 2 -ar 44100 theshow.pcm
    SDL_AudioSpec audioSpec;  //设置audioSpec
    audioSpec.channels = 2;
    audioSpec.format = AUDIO_U8;
    audioSpec.samples = 1024;
    audioSpec.silence = 0;
    audioSpec.freq = 44100;
    audioSpec.callback = fill_data_buffer;
    if (SDL_OpenAudio(&audioSpec,NULL) < 0){  //打开音频
        printf("fail to open audio\n");
        return -1;
    }
    SDL_PauseAudio(0);  //播放
    key_thread = new std::thread(key);
    while(running){
      
        while(audio_len > 0)
        {
            SDL_Delay(1);
        }
        audio_len = dur;
    }
    std::cout << "join"<<std::endl;
    key_thread->join();
    //释放申请的资源
    SDL_Quit();
    std::cout << "quit"<<std::endl;
    return 0;
}
