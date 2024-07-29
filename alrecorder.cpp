// #include <OpenAL/al.h>
// #include <OpenAL/alc.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <iostream>
#include <type_traits>
#include <cstdlib>
#include <thread>
#include "dbg.hpp"


//ALint sample;
//pkg-config --cflags --libs openal
//https://stackoverflow.com/questions/3056113/recording-audio-with-openal
//https://github.com/kcat/openal-soft/blob/master/examples/alrecord.c
int demo(){
    const int SRATE = 44100;
    const int SSIZE = 1024;
    ALCint sample;
    ALbyte buffer[22050];
    alGetError();
    ALCdevice *device = alcCaptureOpenDevice(NULL, SRATE, AL_FORMAT_STEREO16, SSIZE);
    if (alGetError() != AL_NO_ERROR) {
         std::cerr << "alGetError" <<std::endl;
        return 0;
    }
    alcCaptureStart(device);
    while (true) {
     
        alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, (ALCsizei)sizeof(ALint), &sample);
        alcCaptureSamples(device, (ALCvoid *)buffer, sample);

        // ... do something with the buffer 
    }

    alcCaptureStop(device);
    alcCaptureCloseDevice(device);
}
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
int main(int argc, char *argv[]) {
    alinit();
    // clear_alc_errors();
    std::cout << "start recorder" << std::endl;
    // alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER)
    int fmt = AL_FORMAT_STEREO16;
    ALCdevice *device = alcCaptureOpenDevice(
        nullptr, 44100, fmt, 4096);
    if(!device){
        std::cout << "capture device error" << std::endl;
        std::exit(-1);
    }
    std::printf("opened:%s ,al error:",alcGetString(device,ALC_CAPTURE_DEVICE_SPECIFIER));
    std::cout << alGetError()<<",alc err:"
    << alcGetError(device)  << std::endl;
    alcCall(alcCaptureStart,device);
    ALCint sample = -1;
    //mFrameSize = mChannels*mBits/8
    //b/s = sampleRate*mFrameSize
    //buf_size = mFrameSize * sample
    ALbyte* buffer = nullptr;
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
    int read_buf_size = 0;
    while (true) {
        // alcCall(alcGetIntegerv,device, ALC_CAPTURE_SAMPLES, (ALCsizei)sizeof(ALint), &sample);
        // alcCall(alcCaptureSamples,device, (ALCvoid *)buffer, sample);
        alcCall(alcGetIntegerv,device, ALC_CAPTURE_SAMPLES, 1, &sample);
        if(sample<1){
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        auto alloc = [&](){
            int size = frame_size*sample;
            buffer = new int8_t[size];
            read_buf_size = size;
            std::printf("alloc buf:%d\n",size);
        };
        if(!buffer){
            alloc();
        }else if(sample>read_buf_size){
            delete[] buffer;
            alloc();
        }else{
            alcCall(alcCaptureSamples,device, (ALCvoid *)buffer, sample);
        }
        float result = 0;
        for(int i=0;i<sample;i++){
            result+=buffer[i];
        }
        if(result>0.1f){
            std::printf("result %f,sample %d\n",result,sample);
            std::exit(0);
        }
    }
    if(buffer)delete[] buffer;
    alcCall(alcCaptureStop,device);
    alcCall(alcCaptureCloseDevice,device);
    return 0;
}
