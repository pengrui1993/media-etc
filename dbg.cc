#include"dbg.hpp"

void clear_alc_errors(){
    while(ALC_NO_ERROR!=alcGetError(NULL));
}
void clear_al_errors(){
    while(AL_NO_ERROR!=alGetError());
}
void check_alc_errors(ALPath filename, ALLine line,ALCdevice* device){
    ALCenum error = alcGetError(device);
    if(error==ALC_NO_ERROR)return;
    std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n";
    switch(error){
        case ALC_INVALID_DEVICE:
            std::cerr << "ALC_INVALID_DEVICE" << std::endl;
        break;
        case ALC_INVALID_ENUM:
            std::cerr << "ALC_INVALID_ENUM" << std::endl;
        break;
        case ALC_INVALID_VALUE:
            std::cerr << "ALC_INVALID_VALUE" << std::endl;
        break;
        default:
            std::cerr << "ALC error code:"<<std::hex<<error << std::endl;
    }
#if (DEBUG)
    std::exit(0);
#endif
}
void check_al_errors(ALPath filename, ALLine line)
{
    ALCenum error = alGetError();
    if (error != AL_NO_ERROR)
    {
        std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n";
        switch (error)
        {
        case AL_INVALID_NAME:
            std::cerr << "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function";
            break;
        case AL_INVALID_ENUM:
            std::cerr << "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function";
            break;
        case AL_INVALID_VALUE:
            std::cerr << "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function";
            break;
        case AL_INVALID_OPERATION:
            std::cerr << "AL_INVALID_OPERATION: the requested operation is not valid";
            break;
        case AL_OUT_OF_MEMORY:
            std::cerr << "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory";
            break;
        default:
            std::cerr << "UNKNOWN AL ERROR: " << error;
        }
        std::cerr << std::endl;
#if (DEBUG)
    std::exit(0);
#endif
    }
}

void alc_print_devices(){
       std::cout << "default device name:" 
    << alcGetString(NULL, ALC_DEFAULT_ALL_DEVICES_SPECIFIER)
    << std::endl;
    const auto* ds = alcCall(alcGetString,nullptr,ALC_ALL_DEVICES_SPECIFIER);
    using Type = std::remove_const<std::remove_pointer<decltype(ds)>::type>::type;
    Type* p= const_cast<Type*>(ds);
    bool print = true;
    while(true){
        if(p&&print){
            std::cout << p << std::endl;
            print = false;
        }
        p++;
        if(!p[0])print = true;
        if(!p[0]&&!p[1])break;

    }
}