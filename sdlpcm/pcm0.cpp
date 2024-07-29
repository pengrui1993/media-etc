#include <iostream>
#include <thread>
#include <SDL.h>
//音频块，音频的长度，以及当前播放位置
static Uint8 * audioChunk;
static Uint32 audio_len;
static Uint8 *audio_pos;
static bool showed = false;
//回调函数
void fill_data_buffer(void *userData,Uint8 * stream,int len)
{
    if(!showed){
        std::cout << "buf.size:"<<len<<std::endl;
        std::cout << "callback:"<< std::this_thread::get_id() << std::endl;
        showed=true;
    }
    SDL_memset(stream,0,len); //将stream置0
    if (audio_len  == 0)
        return;
    len = len >audio_len? audio_len: len;
    //混音处理
    SDL_MixAudio(stream,audio_pos,len,SDL_MIX_MAXVOLUME);
    audio_pos  += len;
    audio_len -= len;
}
int main(int argc,char** argv)
{
    std::cout << "main:" <<std::this_thread::get_id() << std::endl;

    if (SDL_Init(SDL_INIT_AUDIO))
    {
        std::cerr << "fail to initialize the SDL - " << SDL_GetError() << std::endl;
        return -1;
    }
    const int frame_size = 4096;
    //ffplay -f s16le -ac 2 -ar 44100 theshow.pcm
    //设置audioSpec
    SDL_AudioSpec audioSpec;
    audioSpec.channels = 2;
    audioSpec.format = AUDIO_S16SYS;
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
    //打开文件
    FILE *fp = fopen(argv[1],"rb");
    if (fp == NULL)
    {
        printf("fail to load image\n");
        return -1;
    }
    char  *buffer = (char*)malloc(frame_size);
    int data_count = 0;
    //播放
    SDL_PauseAudio(0);
    bool running = true;
    int count = 0;
    SDL_Event evt;
    while(running)
    {
	    if(count++>=100){
            running = false;
            std::cin.get();
        }
        //读取数据
        if (fread(buffer,1,frame_size,fp) != frame_size)
        {
            fseek(fp,0,SEEK_SET);
            fread(buffer,1,frame_size,fp);
            data_count = 0;
        }
		
        //printf("Playing %10d Bytes data.\n",data_count);
        data_count += frame_size;
        //设置音频缓冲区的数据
        audioChunk = (Uint8*)buffer;
        audio_len = frame_size;
        audio_pos = audioChunk;
        //延迟1ms
        while(audio_len > 0)
        {
            SDL_Delay(1);
        }

    }
    //释放申请的资源
    SDL_Quit();
    free(buffer);
    fclose(fp);
    return 0;
}
