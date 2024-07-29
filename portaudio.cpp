#include<iostream>
#include<cstdlib>
#include<portaudio.h>

//pkg-config --cflags --libs portaudio-2.0
//pkg-config --cflags --libs portaudiocpp
/*
pkg-config --cflags --libs portaudio-2.0
-I/opt/homebrew/Cellar/portaudio/19.7.0/include 
-L/opt/homebrew/Cellar/portaudio/19.7.0/lib 
-lportaudio 
-framework CoreAudio -framework AudioToolbox 
-framework AudioUnit -framework CoreFoundation 
-framework CoreServices


pkg-config --cflags --libs portaudiocpp
-I/opt/homebrew/Cellar/portaudio/19.7.0/include 
-L/opt/homebrew/Cellar/portaudio/19.7.0/lib 
-lportaudiocpp -lportaudio 
-framework CoreAudio -framework AudioToolbox 
-framework AudioUnit -framework CoreFoundation 
-framework CoreServices

*/
///opt/homebrew/opt/openal-soft/lib/pkgconfig:$PKG_CONFIG_PATH

#define SAMPLE_RATE (44100)
typedef struct
{
    float left_phase;
    float right_phase;
}   
paTestData;
static paTestData data;
/* This routine will be called by the PortAudio engine when audio is needed.
 * It may called at interrupt level on some machines so don't do anything
 * that could mess up the system like calling malloc() or free().
*/ 
static int play_cb( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    /* Cast data passed through stream to our structure. */
    paTestData *data = (paTestData*)userData; 
    float *out = (float*)outputBuffer;
    unsigned int i;
    (void) inputBuffer; /* Prevent unused variable warning. */
    
    for( i=0; i<framesPerBuffer; i++ )
    {
         *out++ = data->left_phase;  /* left */
         *out++ = data->right_phase;  /* right */
        /* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
        data->left_phase += 0.01f;
        /* When signal reaches top, drop back down. */
        if( data->left_phase >= 1.0f ) data->left_phase -= 2.0f;
        /* higher pitch so we can distinguish left and right. */
        data->right_phase += 0.03f;
        if( data->right_phase >= 1.0f ) data->right_phase -= 2.0f;
    }
    return 0;
}
bool support(const PaStreamParameters *inputParameters // nullable if outputParameters not null
    ,const PaStreamParameters *outputParameters //nullable if inputParameters not null
    ,double desiredSampleRate
){
        PaError err;
        err = Pa_IsFormatSupported( inputParameters, outputParameters, desiredSampleRate );
        return err == paFormatIsSupported ;
}
/*
PaDeviceInfo
int     structVersion
const char *    name
PaHostApiIndex  hostApi
int     maxInputChannels
int     maxOutputChannels
PaTime  defaultLowInputLatency
PaTime  defaultLowOutputLatency
PaTime  defaultHighInputLatency
PaTime  defaultHighOutputLatency
double  defaultSampleRate
*/
int printDeviceInfo(){
    auto numDevices = Pa_GetDeviceCount();
    if( numDevices < 0 ){
        printf( "ERROR: Pa_GetDeviceCount returned 0x%x\n", numDevices );
        return numDevices;
    }
    const PaDeviceInfo *d;
    for(int i=0; i<numDevices; i++ ){
        d = Pa_GetDeviceInfo( i );
        std::printf("device name:%s\n"
            "\tin nch:%d,out nch:%d\n"
        ,d->name,d->maxInputChannels,d->maxOutputChannels);
        /* device name:MacBook Pro Microphone
                in nch:1,out nch:0
        device name:MacBook Pro Speakers
                in nch:0,out nch:2*/
    }
    return numDevices;
}
void initParam(int inChan,int inDevNum,int outChan,int outDevNum){
    PaStreamParameters outputParameters;
    PaStreamParameters inputParameters;
    bzero( &inputParameters, sizeof( inputParameters ) ); //not necessary if you are filling in all the fields
    inputParameters.channelCount = inChan;
    inputParameters.device = inDevNum;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inDevNum)->defaultLowInputLatency ;
    inputParameters.hostApiSpecificStreamInfo = NULL; //See you specific host's API docs for info on using this field
    bzero( &outputParameters, sizeof( outputParameters ) ); //not necessary if you are filling in all the fields
    outputParameters.channelCount = outChan;
    outputParameters.device = outDevNum;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outDevNum)->defaultLowOutputLatency ;
    outputParameters.hostApiSpecificStreamInfo = NULL; //See you specific host's API docs for info on using this field
    if(support(&inputParameters,nullptr,44100)){}
    if(support(nullptr,&outputParameters,44100)){}
}
PaError openDftStream(PaStream **stream){
    return Pa_OpenDefaultStream(stream,
                                0,          /* no input channels */
                                2,          /* stereo output */
                                paFloat32,  /* 32 bit floating point output */
                                SAMPLE_RATE,
                                256,        /* frames per buffer, i.e. the number
                                                   of sample frames that PortAudio will
                                                   request from the callback. Many apps
                                                   may want to use
                                                   paFramesPerBufferUnspecified, which
                                                   tells PortAudio to pick the best,
                                                   possibly changing, buffer size.*/
                                play_cb, /* this is your callback function */
                                &data ); /*This is a pointer that will be passed to
                                                   your callback*/
}
namespace{
    auto NUM_SECONDS = 1;
    struct ctx{
        PaStream* stream;
        PaStreamParameters inputParameters{0};
        double srate{44100};
        unsigned long framesPerBuffer = paFramesPerBufferUnspecified ; //could be paFramesPerBufferUnspecified
    } c;
}
static int record_cb( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData ){
        (void)outputBuffer;
        ctx& c = *(ctx*)userData;
        float *in = (float*)inputBuffer;
        for(int i=0; i<framesPerBuffer; i++ ){
            // float *out = (float*)outputBuffer;
            // *out++ = data->left_phase;  /* left */
            // *out++ = data->right_phase;  /* right */
            // /* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
            // data->left_phase += 0.01f;
            // /* When signal reaches top, drop back down. */
            // if( data->left_phase >= 1.0f ) data->left_phase -= 2.0f;
            // /* higher pitch so we can distinguish left and right. */
            // data->right_phase += 0.03f;
            // if( data->right_phase >= 1.0f ) data->right_phase -= 2.0f;
        }
}
//https://files.portaudio.com/docs/v19-doxydocs/tutorial_start.html
void openRecoderStream(PaStream **stream){
    auto numDevices = Pa_GetDeviceCount();
    if( numDevices < 0 ){
        printf( "ERROR: Pa_GetDeviceCount returned 0x%x\n", numDevices );
        std::exit(-1);
    }
    const PaDeviceInfo *d;
    for(int i=0; i<numDevices; i++ ){
        d = Pa_GetDeviceInfo( i );
        if(d->maxInputChannels>0){
            PaStreamParameters& inputParameters = c.inputParameters;
            inputParameters.channelCount = d->maxInputChannels;
            inputParameters.device = i;
            inputParameters.hostApiSpecificStreamInfo = NULL;
            inputParameters.sampleFormat = paFloat32;
            inputParameters.suggestedLatency = Pa_GetDeviceInfo(i)->defaultLowInputLatency ;
            inputParameters.hostApiSpecificStreamInfo = NULL; //See you specific host's API docs for info on using this field
            break;
        }
        /* device name:MacBook Pro Microphone
                in nch:1,out nch:0
        device name:MacBook Pro Speakers
                in nch:0,out nch:2*/
    }
    int err = Pa_OpenStream(
                    stream,
                    &c.inputParameters,
                    nullptr,
                    c.srate,
                    c.framesPerBuffer,
                    paNoFlag, //flags that can be used to define dither, clip settings and more
                    nullptr, //your callback function
                    (void *)&c ); //data to be passed to callback. In C++, it is frequently (void *)this
    if(err!=paNoError){
        std::printf("Pa_OpenStream error\n");
        std::exit(-1);
    }

}
int play(){
  
    auto err = Pa_Initialize();
    decltype(c.stream) stream = c.stream;
    if( err != paNoError ){
        std::printf("init error\n");
        goto error;
    }
    printDeviceInfo();
    // initParam();
    /* Open an audio I/O stream. */
    if( err != paNoError ) goto error;
    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;
    /* Sleep for several seconds. */
    Pa_Sleep(NUM_SECONDS*1000);
    err = Pa_StopStream( stream );
    if( err != paNoError ) goto error;
    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto error;
    err = Pa_Terminate( );
    if( err != paNoError ) goto error;

    std::exit(0);
    error:
        std::printf("PortAudio error: %s\n", Pa_GetErrorText( err ));
        std::exit(-1);
    return 0;
}
int main(){
    return play();
}