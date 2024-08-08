// #include <OpenAL/al.h>
// #include <OpenAL/alc.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <iostream>
#include <type_traits>
#include <cstdlib>
#include <thread>
#include <fstream>
#include "dbg.hpp"
/*
TODO noise problem later care
 */
static void alinit(){
 if(alcIsExtensionPresent(NULL, "ALC_SOFT_loopback") == ALC_FALSE) {
        std::fputs("Your OpenAL implementation doesn't support the "
              "\"ALC_SOFT_loopback\" extension, required for this test. "
              "Sorry.\n", stderr);
        std::exit(EXIT_FAILURE);
    }
    ALCint alc_major, alc_minor;
    alcGetIntegerv(NULL, ALC_MAJOR_VERSION, 1, &alc_major);
    alcGetIntegerv(NULL, ALC_MINOR_VERSION, 1, &alc_minor);
    std::printf("major:%d,minor:%d\n",alc_major,alc_minor);
    if(alc_major<1 || (alc_major==1 && alc_minor<1))
        std::fputs("Warning : ALC_SOFT_loopback has been written against "
              "the OpenAL 1.1 specification.\n", stderr);
}

//ALint sample;
//pkg-config --cflags --libs openal
//https://stackoverflow.com/questions/3056113/recording-audio-with-openal

int demo(){
    ALCint sample;
    // ALbyte buffer[22050];//small to get crash
    ALbyte buffer[5*7526];
    bzero(buffer,sizeof(buffer)/sizeof(buffer[0]));
    alcCall(alcGetError,nullptr);
    ALCdevice *device = alcCaptureOpenDevice(nullptr, 44100, AL_FORMAT_STEREO16, 1024);
    if(!device){
        std::printf("init device error\n");
        std::exit(-1);
    }
    if (alcCall(alcGetError,device) != ALC_NO_ERROR) {
         std::cerr << "alcGetError" <<std::endl;
        return 0;
    }
    alcCall(alcCaptureStart,device);
    while (true) {
        // constexpr auto size = (ALCsizei)sizeof(ALint);
        constexpr auto size = 1;
        alcCall(alcGetIntegerv,device, ALC_CAPTURE_SAMPLES,size, &sample);
        if(sample<1){
             std::this_thread::sleep_for(std::chrono::seconds(1));
             continue;
        }
        float value = 0;
        for(int i=0;i<size*sample;i++)value+=buffer[i];
        std::fprintf(stderr,"sample:%d,value:%f ",sample,value);
        alcCall(alcCaptureSamples,device, (ALCvoid *)buffer, sample);
        // ... do something with the buffer 
    }
    alcCaptureStop(device);
    alcCaptureCloseDevice(device);
}

//https://github.com/kcat/openal-soft/blob/master/examples/alrecord.c
int frame_size_from(int fmt){
    int frame_size;
    switch(fmt){
        case AL_FORMAT_MONO8:frame_size=1*(8/8);break;
        case AL_FORMAT_MONO16:frame_size=1*(16/8);break;
        case AL_FORMAT_STEREO8:frame_size=2*(8/8);break;
        case AL_FORMAT_STEREO16:frame_size=2*(16/8);break;
        default:
            std::printf("invalid fmt");
            std::exit(-1);
    }
    return frame_size;
}
//int t1(int argc, char *argv[]);
int t1() {
    alinit();
    // clear_alc_errors();
    //int fmt = AL_FORMAT_STEREO16;
    int fmt = AL_FORMAT_MONO8;
    ALCdevice *device = alcCaptureOpenDevice(
        nullptr, 44100, fmt, 4096);
    if(!device){
        std::cout << "capture device error" << std::endl;
        std::exit(-1);
    }
    std::printf("opened:%s ,al error:",alcGetString(device,ALC_CAPTURE_DEVICE_SPECIFIER));
    std::cout << alGetError()<<",alc err:"<< alcGetError(device)  << std::endl;
    alcCall(alcCaptureStart,device);
    ALCint sample = -1;
    //mFrameSize = mChannels*mBits/8
    //b/s = sampleRate*mFrameSize
    //buf_size = mFrameSize * sample
    ALbyte* buffer = nullptr;
    int read_buf_size = 0;
    std::ofstream f;
    f.open("/tmp/cpp.pcm",std::ios::trunc);
    int count = 0;
    int frame_size = frame_size_from(fmt);
    while (true) {
        if(count++>1000)break;
        alcCall(alcGetIntegerv,device, ALC_CAPTURE_SAMPLES, 1, &sample);
        if(sample<1){
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        auto alloc = [&](){
            buffer = new int8_t[sample];
            read_buf_size = sample;
            std::printf("alloc buf:%d\n",sample);
        };
        if(!buffer){
            alloc();
        }else if(sample>read_buf_size){
            delete[] buffer;
            alloc();
        }else{
            std::memset(buffer,0,frame_size*sample);
            alcCall(alcCaptureSamples,device, (ALCvoid *)buffer, sample);
            for(int i=0;i<sample;i++)f<<buffer[i];
        }
    }
    f.close();
    std::cout <<"ALCdevice *device = alcCaptureOpenDevice(nullptr, 44100, AL_FORMAT_MONO8, 4096)"<<std::endl
            <<"ffplay -f u8 -ch_layout mono -ar 44100 /tmp/cpp.pcm"<< std::endl
            ;
    if(buffer)delete[] buffer;
    alcCall(alcCaptureStop,device);
    alcCall(alcCaptureCloseDevice,device);
    return 0;
}
/*
about al device runtime managment
https://openal.org/pipermail/openal/2016-September/000550.html
*/
int main(){
    return t1();
}