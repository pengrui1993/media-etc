#include <AL/alc.h>
#include <iostream>
#include <fstream>
#include <vorbis/vorbisfile.h>
#include <functional>
#include <AL/al.h>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <ratio>
#define DEBUG 1
#define SLOG 1
#if (DEBUG)
#define alCall(function, ...) alCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)
#else
#define alCall(function, ...) function(__VA_ARGS__)
#endif
#define alcCall(function, device, ...) alcCallImpl(__FILE__, __LINE__, function, device, __VA_ARGS__)
void check_al_errors(const std::string &filename, const std::uint_fast32_t line)
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
    }
}
template <typename alFunction, typename... Params>
auto alCallImpl(const char *filename, const std::uint_fast32_t line, alFunction function, Params... params)
    -> typename std::enable_if<std::is_same<void, decltype(function(params...))>::value, decltype(function(params...))>::type
{
    function(std::forward<Params>(params)...);
    check_al_errors(filename, line);
}

template <typename alFunction, typename... Params>
auto alCallImpl(const char *filename, const std::uint_fast32_t line, alFunction function, Params... params)
    -> typename std::enable_if<!std::is_same<void, decltype(function(params...))>::value, decltype(function(params...))>::type
{
    auto ret = function(std::forward<Params>(params)...);
    check_al_errors(filename, line);
    return ret;
}
struct OggAudioPlayer
{
    static const std::size_t NUM_BUFFERS = 2;
    static const ALsizei BUFFER_SIZE = 10 * 1024;
    typedef char byte;

public:
    bool load(std::string path)
    {
        filepath = path;
        file.open(path, std::ios::binary);
        if (!file)
        {
            std::cerr << "file cannot open:" << path << std::endl;
            return false;
        }
        file.seekg(0, std::ios::beg);
        size = sizeAndReset(file);
        // std::cout << "size:" << size << std::endl;
        // std::cout << "pos:" << file.tellg() << std::endl;
        { // 1.seek 2.repeated read 3.seek 4.tell
            oggcbs.seek_func = oggSeek;
            oggcbs.read_func = oggRead;
            oggcbs.tell_func = oggTell;
            oggcbs.close_func = oggClose;
        }
        if (ov_open_callbacks(this, &oggFile, 0, -1, oggcbs) < 0)
        {
            std::cerr << "cb err" << std::endl;
            return false;
        }
        // std::cout << "cb ok,size:" << consumed << std::endl;//cb ok,size:10182
        auto info = ov_info(&oggFile, -1);
        channels = info->channels;
        sampleRate = info->rate;
        bitPerSample = 16; // if 8 then AL_FORMAT_MONO8 or AL_FORMAT_STEREO8
        duration = ov_time_total(&oggFile, -1);
        switch (channels)
        {
        case 1:
            fmt = AL_FORMAT_MONO16;
            break;
        case 2:
            fmt = AL_FORMAT_STEREO16;
            break;
        default:
            std::cerr << "invalid audio format, channels:" << channels << std::endl;
            return false;
        }
        { // al call
            /*
                alCall(alGenSources, 1, &audioData.source);
    alCall(alSourcef, audioData.source, AL_PITCH, 1);
    alCall(alSourcef, audioData.source, AL_GAIN, DEFAULT_GAIN);
    alCall(alSource3f, audioData.source, AL_POSITION, 0, 0, 0);
    alCall(alSource3f, audioData.source, AL_VELOCITY, 0, 0, 0);
    alCall(alSourcei, audioData.source, AL_LOOPING, AL_FALSE);
    alCall(alGenBuffers, NUM_BUFFERS, &audioData.buffers[0]);
            */
            alCall(alGenSources, 1, &source);
            alCall(alSourcef, source, AL_PITCH, pitch);
            alCall(alSourcef, source, AL_GAIN, gain);
            alCall(alSource3f, source, AL_POSITION, p1, p2, p3);
            alCall(alSource3f, source, AL_VELOCITY, v1, v2, v3);
            alCall(alSourcei, source, AL_LOOPING, loop);
            alCall(alGenBuffers, NUM_BUFFERS, &nbuf[0]);
        }
        isOver = false;
        isPlaying = false;
        return true;
    }
    bool over()
    {
        if (consumed == size)
        {
            isOver = true;
            std::cout << "over done" << std::endl;
        }
        return isOver;
    }
    void play()
    {
        byte buffer[BUFFER_SIZE]{0};
        if (!isPlaying)
        {
            for (size_t i = 0; i < NUM_BUFFERS; i++)
            {
                size_t index = 0;
                while (index < BUFFER_SIZE)
                {
                    long res;
                    if ((res = ovRead(buffer, i, index)) > 0)
                    {
                        index += res;
                    }
                }
                std::cout << "index:" << index << ",buffer.id:" << nbuf[i] << std::endl;
                // alCall(alBufferData, audioData.buffers[i], audioData.format, data, dataSoFar, audioData.sampleRate);
                alCall(alBufferData, nbuf[i], fmt, &buffer[0], index, sampleRate);
            }
            //    alCall(alSourceQueueBuffers, audioData.source, NUM_BUFFERS, &audioData.buffers[0]);
            alCall(alSourceQueueBuffers, source, NUM_BUFFERS, nbuf); //&nbuf[0]
            alCall(alSourcePlay, source);
            isPlaying = true;
        }
        else
        {
            ALint buffersProcessed = 0;
            alCall(alGetSourcei, source, AL_BUFFERS_PROCESSED, &buffersProcessed);
            if (buffersProcessed <= 0)
                return;
            ALuint bufferId;
            alCall(alSourceUnqueueBuffers, source, 1, &bufferId);
            //::bzero(buffer, BUFFER_SIZE);
            std::memset(buffer, 0, BUFFER_SIZE);
            size_t index = 0;
            while (index < BUFFER_SIZE)
            {
                long res;
                if ((res = ovRead(buffer, -1, index)) > 0)
                {
                    index += res;
                }
                else if (res < 0)
                {
                    exit(-2);
                }
                else // 0
                {
                    exit(-3);
                }
            }
            alCall(alBufferData, bufferId, fmt, &buffer[0], index, sampleRate);
            alCall(alSourceQueueBuffers, source, 1, &bufferId);
            if (!remoteIsPlaying())
            { // alCall(alSourceStop, source);
                alCall(alSourcePlay, source);
            }
        }
    }
    ~OggAudioPlayer()
    {
        if (file.is_open())
            file.close();
        if (source > 0)
        {
            alDeleteSources(1, &source);
            source = 0;
        }
        if (nbuf[0] > 0)
        {
            alDeleteBuffers(NUM_BUFFERS, nbuf);
            std::memset(nbuf, 0, NUM_BUFFERS);
        }
    }
    bool remoteIsPlaying() const
    {
        /*
        ALint state;
        alCall(alGetSourcei, audioData.source, AL_SOURCE_STATE, &state);*/
        ALint state;
        alCall(alGetSourcei, source, AL_SOURCE_STATE, &state);
        const bool res = AL_PLAYING == state;

        return res;
    }

private:
    long ovRead(byte *buffer, size_t i, size_t index)
    {
        auto res = ov_read(&oggFile, &buffer[index], BUFFER_SIZE - index, 0, 2, 1, &oggCurrentSection);
        if (res <= 0)
        {
            switch (res)
            {
            case 0:
                std::cerr << "read err,buffer:" << i << ",end of file" << std::endl;
                return res;
            case OV_HOLE:
                std::cerr << "read err,buffer:" << i << ",hole" << std::endl;
                return res;
            case OV_EBADLINK:
                std::cerr << "read err,buffer:" << i << ",bad link" << std::endl;
                return res;
            case OV_EINVAL:
                std::cerr << "read err,buffer:" << i << ",invalid" << std::endl;
                return res;
            default:
                std::cerr << "read err,buffer:" << i << ",unknown error:" << res << std::endl;
                exit(-1);
            }
        }
        return res;
    }
    static int oggClose(void *ds)
    {
        std::cout << "ogg close" << std::endl;
        return 0;
    }
    static size_t oggRead(void *ptr, size_t size, size_t nmemb, void *ds)
    {
#if SLOG
        auto now = std::chrono::system_clock::now();
        auto tt = std::chrono::system_clock::to_time_t(now);
        auto str = ctime(&tt);
        std::cout << "ogg read:" << now.time_since_epoch().count() << std::endl;
#endif
        OggAudioPlayer *p = reinterpret_cast<OggAudioPlayer *>(ds);
        auto length = size * nmemb;
        if (p->consumed + length > p->size)
        { // last read
            const size_t fileSize = p->size;
            length = fileSize - p->consumed;
        }
        std::ifstream &ref = p->file;
        if (!ref.is_open())
        {
            std::cout << "file not open" << std::endl;
            return 0;
        }
        if (!ref.good())
        {
#define bts(e) e ? "y" : "n"
            std::printf("warning fstream some error, eof:%s,fail:%s,bad:%s\n", bts(ref.eof()), bts(ref.fail()), bts(ref.bad()));
#undef bts
            ref.clear();
        }

        byte *buf = new byte[length];
        ref.seekg(p->consumed);
        if (!ref.read(&buf[0], length))
        {
            std::cerr << "file read error,pos:" << p->consumed << std::endl;
            return 0;
        }
        p->consumed += length;

        std::memcpy(ptr, &buf[0], length);
        delete[] buf;
#if SLOG
        std::cout << "ogg read length:" << length << std::endl;
#endif
        return length;
    }
    static int oggSeek(void *ds, ogg_int64_t offset, int whence)
    {
        std::cout << "ogg seek" << std::endl;
        OggAudioPlayer *p = reinterpret_cast<OggAudioPlayer *>(ds);
        switch (whence)
        {
        case SEEK_CUR:
            p->consumed += offset;
            break;
        case SEEK_END:
            p->consumed = p->size - offset;
            break;
        case SEEK_SET:
            p->consumed = offset;
            break;
        default:
            return -1;
        }
        if (p->consumed < 0)
        {
            std::cerr << "consumed less then zero" << std::endl;
            return -1;
        }
        if (p->consumed > p->size)
        {
            std::cerr << "consumed more then file's size" << std::endl;
            return -1;
        }
        return 0;
    }
    static long oggTell(void *ds)
    {
        std::cout << "ogg tell" << std::endl;
        OggAudioPlayer *p = reinterpret_cast<OggAudioPlayer *>(ds);
        return p->consumed;
    }
    template <typename RET, typename PARAM>
    auto call(std::function<RET(PARAM)> f) -> PARAM
    {
        return f();
    }
    auto sizeAndReset(std::ifstream &file) -> decltype(file.tellg())
    {
        auto tmp = file.tellg();
        file.seekg(0, std::ios::end);
        auto res = file.tellg();
        file.seekg(tmp);
        //  std::cout << "file size:" << res << std::endl; // file size:116492
        return res;
    }
    std::streampos sizeAndReset()
    {
        auto tmp = file.tellg();
        file.seekg(0, std::ios::beg);
        file.ignore(std::numeric_limits<std::streamsize>::max());
        auto res = file.gcount();
        // std::cout << "file size2:" << file.gcount() << std::endl; // file size2:116492
        file.clear();
        file.seekg(tmp);
        return res;
    }

private:
    std::string filepath;
    std::ifstream file;
    std::streampos size;
    ALCsizei consumed{0};
    ov_callbacks oggcbs;
    OggVorbis_File oggFile;
    int channels{-1};
    long sampleRate{-1L};
    uint8_t bitPerSample{0};
    size_t duration{0};
    bool isOver{true}, isPlaying{false};
    int oggCurrentSection{0};

private:
    ALuint source{0};
    ALfloat pitch{1.f}, gain{1.f};
    ALint loop{AL_FALSE};
    ALfloat p1{0}, p2{0}, p3{0};
    ALfloat v1{0}, v2{0}, v3{0};
    ALenum fmt{AL_FORMAT_STEREO8};
    ALuint nbuf[NUM_BUFFERS]{0};
};
/*
Listener Properties
For every context, there is automatically one Listener object. The alListener[f, 3f, fv, i] and
alGetListener[f, 3f, fv, i] families of functions can be used to set or retrieve the following listener
properties:
Property Data Type Description
AL_GAIN f, fv “master gain”
value should be positive
AL_POSITION fv, 3f, iv, 3i X, Y, Z position
AL_VELOCITY fv, 3f, iv, 3i velocity vector
AL_ORIENTATION fv, iv orientation expressed as “at” and “up” vectors
*/
void listener(ALfloat pos[3], ALfloat vel[3], ALfloat ori[6])
{
    // alListenerfv(AL_POSITION, pos);
    alCall(alListenerfv, AL_POSITION, pos);
    // alListenerfv(AL_VELOCITY, vel);
    alCall(alListenerfv, AL_VELOCITY, vel);
    // alListenerfv(AL_ORIENTATION, ori);
    alCall(alListenerfv, AL_ORIENTATION, ori);
}
void testchrono()
{
    using std::chrono::system_clock;
    std::chrono::duration<int, std::ratio<60 * 60 * 24>> one_day(1);
    system_clock::time_point today = system_clock::now();
    system_clock::time_point tomorrow = today + one_day;
    std::time_t tt;
    tt = system_clock::to_time_t(today);
    std::cout << "today is: " << ctime(&tt);
    tt = system_clock::to_time_t(tomorrow);
    std::cout << "tomorrow will be: " << ctime(&tt);
}
int main()
{
    auto aldevice = alcOpenDevice(nullptr);
    if (!aldevice)
    {
        std::cerr << "open default device error" << std::endl;
        return -1;
    }
    auto alctx = alcCreateContext(aldevice, nullptr);
    if (!alcMakeContextCurrent(alctx))
    {
        std::cerr << "make context error" << std::endl;
        return -1;
    }
    if (alcGetContextsDevice(alctx) == aldevice)
    {
        std::cout << "same device\n";
    }
    if (alcGetCurrentContext() == alctx)
    {
        std::cout << "current context same as" << std::endl;
    }
    OggAudioPlayer player;
    player.load("./nice_music.ogg");
    while (!player.over())
    {
        player.play();
    }
    if (aldevice)
    {
        if (!alcCloseDevice(aldevice))
        {
            std::cerr << "close defice error" << std::endl;
        }
    }

    return 0;
}