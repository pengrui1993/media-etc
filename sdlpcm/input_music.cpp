#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <SDL.h>
#define HARF_SQ(n) powf(2,n/12.0)
#define TEST_BUF_SIZE 2048
#define MIN(a,b) ((a>b)?b:a)
#define SAMPLE_FREQ (44100/2)
#define PI 3.14159

static bool running = true;
static int count = 0;
//音频块，音频的长度，以及当前播放位置
using u8 = Uint8;
using u64 = uint64_t;
static Uint8 * audioChunk;
static Uint32 audio_len;
static Uint8 *audio_pos;
static u64 tick = 0;
static const double init_freq = 220*2*(1/powf(2,3/12.0));
static double freq = init_freq;
static u8 dur = 5; //duration
static std::unordered_map<char,double> freq_map{
     {'z',HARF_SQ(0)}
    ,{'s',HARF_SQ(1)}
    ,{'x',HARF_SQ(2)}
    ,{'d',HARF_SQ(3)}
    ,{'c',HARF_SQ(4)}
    ,{'v',HARF_SQ(5)}
    ,{'g',HARF_SQ(6)}
    ,{'b',HARF_SQ(7)}
    ,{'h',HARF_SQ(8)}
    ,{'n',HARF_SQ(9)}//440
    ,{'j',HARF_SQ(10)}
    ,{'m',HARF_SQ(11)}
    ,{',',2}
};
static bool show_crab_noise = false;
static u8 noise(){
    // static const int freq = 1;
    // static double omiga = 2*PI*freq; 
    int _8bit_max = (1<<8)-1;
    double  val = 0.5*sin(2*PI *freq*  (tick++)*(1.0/SAMPLE_FREQ))+0.5;// [0,1]
    if(show_crab_noise)tick%=SAMPLE_FREQ;
    return (u8)(_8bit_max*val);
}
void fill_data_buffer(void *userData,Uint8 * stream,int len)
{
    static u8 buf[TEST_BUF_SIZE];
    static Uint8* p;
    SDL_memset(stream,0,len);
    if(0==audio_len)return;
    int buf_len = TEST_BUF_SIZE;
    p = buf;
    len = MIN(buf_len,len);
    for(int i=0;i<len;i++)p[i]=noise();
    SDL_MixAudio(stream,p,len,SDL_MIX_MAXVOLUME);
    audio_len--;
}
//zzxxcczzzzxxcczzccvvbbbbccvvbbbbbnbvcczzbnbvcczzxxbbzzzzxxbbzzzz
static void input(){
    switch(count=std::cin.get()){
        case 'q':running = false;break;
        case 'z':
        case 's':
        case 'x':
        case 'd':
        case 'c':
        case 'v':
        case 'g':
        case 'b':
        case 'h':
        case 'n':
        case 'j':
        case 'm':
        case ',':
            freq = init_freq*freq_map[count];
            std::cout <<"change freq " <<(char)count<<std::endl;
            count = 0;
        break;
        case '\n':break;// ignore newline
        default:
            std::cout <<"q for quit,continue."
                <<"current cmd:"<<(char)count
                <<",num:"<<count<< std::endl;
            count = 0;
            break;
    }
    while(audio_len > 0)
    {
        SDL_Delay(1);
    }
    audio_len = dur;
}

int main(int argc,char** argv)
{
    std::cout << "show play noise? 1:yes,0:no" << std::endl;
    std::cin >> show_crab_noise;
    std::cout << "choose:"<< show_crab_noise << std::endl;
    if (SDL_Init(SDL_INIT_AUDIO)){
        std::cerr << "fail to initialize the SDL - " << SDL_GetError() << std::endl;
        return -1;
    }
    //ffplay -f s16le -ac 2 -ar 44100 theshow.pcm
    //设置audioSpec
    SDL_AudioSpec audioSpec;
    audioSpec.channels = 2;
    audioSpec.format = AUDIO_U8;
    audioSpec.samples = 1024;
    //audioSpec.samples = 1024;
    audioSpec.silence = 0;
    audioSpec.freq = 44100;
    audioSpec.callback = fill_data_buffer;
    //打开音频
    if (SDL_OpenAudio(&audioSpec,NULL) < 0)
    {
        printf("fail to open audio\n");
        return -1;
    }
    SDL_PauseAudio(0);  //播放
    printf("terminal input that: \n");
    std::cout <<"zzxxcczzzzxxcczzccvvbbbbccvvbbbbbnbvcczzbnbvcczzxxbbzzzzxxbbzzzz" << std::endl;
    while(running){

        input();
    }
    //释放申请的资源
    SDL_Quit();
    return 0;
}
