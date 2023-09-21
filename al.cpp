#include <iostream>
#include <cstddef>
#include <cinttypes>
#include <fstream>
#include <string>
#include <AL/al.h>
#include <AL/alc.h>
#include <vorbis/vorbisfile.h>
const std::size_t NUM_BUFFERS = 4;
const ALsizei BUFFER_SIZE = 65536;
const ALfloat DEFAULT_GAIN = 1.f;
struct StreamingAudioData
{
    ALuint buffers[NUM_BUFFERS];
    std::string filename;
    std::ifstream file;
    std::uint8_t channels;
    std::int32_t sampleRate;
    std::uint8_t bitsPerSample;
    ALsizei size;
    ALuint source;
    ALsizei sizeConsumed = 0;
    ALenum format;
    OggVorbis_File oggVorbisFile;
    std::int_fast32_t oggCurrentSection = 0;
    std::size_t duration;
};
#define DEBUG 1
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

std::size_t read_ogg_callback(void *destination, std::size_t size1, std::size_t size2, void *fileHandle)
{
    StreamingAudioData *audioData = reinterpret_cast<StreamingAudioData *>(fileHandle);

    ALsizei length = size1 * size2;

    if (audioData->sizeConsumed + length > audioData->size)
        length = audioData->size - audioData->sizeConsumed;

    if (!audioData->file.is_open())
    {
        audioData->file.open(audioData->filename, std::ios::binary);
        if (!audioData->file.is_open())
        {
            std::cerr << "ERROR: Could not re-open streaming file \"" << audioData->filename << "\"" << std::endl;
            return 0;
        }
    }

    char *moreData = new char[length];

    audioData->file.clear();
    audioData->file.seekg(audioData->sizeConsumed);
    if (!audioData->file.read(&moreData[0], length))
    {
        if (audioData->file.eof())
        {
            audioData->file.clear(); // just clear the error, we will resolve it later
        }
        else if (audioData->file.fail())
        {
            std::cerr << "ERROR: OGG stream has fail bit set " << audioData->filename << std::endl;
            audioData->file.clear();
            return 0;
        }
        else if (audioData->file.bad())
        {
            perror(("ERROR: OGG stream has bad bit set " + audioData->filename).c_str());
            audioData->file.clear();
            return 0;
        }
    }
    audioData->sizeConsumed += length;

    std::memcpy(destination, &moreData[0], length);

    delete[] moreData;

    audioData->file.clear();

    return length;
}

std::int32_t seek_ogg_callback(void *fileHandle, ogg_int64_t to, std::int32_t type)
{
    StreamingAudioData *audioData = reinterpret_cast<StreamingAudioData *>(fileHandle);
    // StreamingAudioData *audioData = static_cast<StreamingAudioData *>(fileHandle);
    if (type == SEEK_CUR)
    {
        audioData->sizeConsumed += to;
    }
    else if (type == SEEK_END)
    {
        audioData->sizeConsumed = audioData->size - to;
    }
    else if (type == SEEK_SET)
    {
        audioData->sizeConsumed = to;
    }
    else
        return -1; // what are you trying to do vorbis?

    if (audioData->sizeConsumed < 0)
    {
        audioData->sizeConsumed = 0;
        return -1;
    }
    if (audioData->sizeConsumed > audioData->size)
    {
        audioData->sizeConsumed = audioData->size;
        return -1;
    }

    return 0;
}
long int tell_ogg_callback(void *fileHandle)
{
    StreamingAudioData *audioData = reinterpret_cast<StreamingAudioData *>(fileHandle);
    return audioData->sizeConsumed;
}
bool create_stream_from_file(const std::string &filename, StreamingAudioData &audioData)
{
    audioData.filename = filename;
    audioData.file.open(filename, std::ios::binary);
    if (!audioData.file.is_open())
    {
        std::cerr << "ERROR: couldn't open file" << std::endl;
        return 0;
    }

    audioData.file.seekg(0, std::ios_base::beg);
    audioData.file.ignore(std::numeric_limits<std::streamsize>::max());
    audioData.size = audioData.file.gcount();
    audioData.file.clear();
    audioData.file.seekg(0, std::ios_base::beg);
    audioData.sizeConsumed = 0;

    ov_callbacks oggCallbacks;
    oggCallbacks.read_func = read_ogg_callback;
    oggCallbacks.close_func = nullptr;
    oggCallbacks.seek_func = seek_ogg_callback;
    oggCallbacks.tell_func = tell_ogg_callback;

    if (ov_open_callbacks(reinterpret_cast<void *>(&audioData), &audioData.oggVorbisFile, nullptr, -1, oggCallbacks) < 0)
    {
        std::cerr << "ERROR: Could not ov_open_callbacks" << std::endl;
        return false;
    }

    vorbis_info *vorbisInfo = ov_info(&audioData.oggVorbisFile, -1);

    audioData.channels = vorbisInfo->channels;
    audioData.bitsPerSample = 16;
    audioData.sampleRate = vorbisInfo->rate;
    audioData.duration = ov_time_total(&audioData.oggVorbisFile, -1);

    alCall(alGenSources, 1, &audioData.source);
    alCall(alSourcef, audioData.source, AL_PITCH, 1);
    alCall(alSourcef, audioData.source, AL_GAIN, DEFAULT_GAIN);
    alCall(alSource3f, audioData.source, AL_POSITION, 0, 0, 0);
    alCall(alSource3f, audioData.source, AL_VELOCITY, 0, 0, 0);
    alCall(alSourcei, audioData.source, AL_LOOPING, AL_FALSE);

    alCall(alGenBuffers, NUM_BUFFERS, &audioData.buffers[0]);

    if (audioData.file.eof())
    {
        std::cerr << "ERROR: Already reached EOF without loading data" << std::endl;
        return false;
    }
    else if (audioData.file.fail())
    {
        std::cerr << "ERROR: Fail bit set" << std::endl;
        return false;
    }
    else if (!audioData.file)
    {
        std::cerr << "ERROR: file is false" << std::endl;
        return false;
    }

    char *data = new char[BUFFER_SIZE];

    for (std::uint8_t i = 0; i < NUM_BUFFERS; ++i)
    {
        std::int32_t dataSoFar = 0;
        while (dataSoFar < BUFFER_SIZE)
        {
            std::int32_t result = ov_read(&audioData.oggVorbisFile, &data[dataSoFar], BUFFER_SIZE - dataSoFar, 0, 2, 1, &audioData.oggCurrentSection);
            if (result == OV_HOLE)
            {
                std::cerr << "ERROR: OV_HOLE found in initial read of buffer " << i << std::endl;
                break;
            }
            else if (result == OV_EBADLINK)
            {
                std::cerr << "ERROR: OV_EBADLINK found in initial read of buffer " << i << std::endl;
                break;
            }
            else if (result == OV_EINVAL)
            {
                std::cerr << "ERROR: OV_EINVAL found in initial read of buffer " << i << std::endl;
                break;
            }
            else if (result == 0)
            {
                std::cerr << "ERROR: EOF found in initial read of buffer " << i << std::endl;
                break;
            }

            dataSoFar += result;
        }

        if (audioData.channels == 1 && audioData.bitsPerSample == 8)
            audioData.format = AL_FORMAT_MONO8;
        else if (audioData.channels == 1 && audioData.bitsPerSample == 16)
            audioData.format = AL_FORMAT_MONO16;
        else if (audioData.channels == 2 && audioData.bitsPerSample == 8)
            audioData.format = AL_FORMAT_STEREO8;
        else if (audioData.channels == 2 && audioData.bitsPerSample == 16)
            audioData.format = AL_FORMAT_STEREO16;
        else
        {
            std::cerr << "ERROR: unrecognised ogg format: " << audioData.channels << " channels, " << audioData.bitsPerSample << " bps" << std::endl;
            delete[] data;
            return false;
        }

        alCall(alBufferData, audioData.buffers[i], audioData.format, data, dataSoFar, audioData.sampleRate);
    }

    alCall(alSourceQueueBuffers, audioData.source, NUM_BUFFERS, &audioData.buffers[0]);

    delete[] data;

    return true;
}
void play_stream(const StreamingAudioData &audioData)
{
    alCall(alSourceStop, audioData.source);
    alCall(alSourcePlay, audioData.source);
}

void update_stream(StreamingAudioData &audioData)
{
    ALint buffersProcessed = 0;
    alCall(alGetSourcei, audioData.source, AL_BUFFERS_PROCESSED, &buffersProcessed);
    std::cout << "buffers processed:" << buffersProcessed << std::endl;
    if (buffersProcessed <= 0)
    {
        return;
    }
    while (buffersProcessed--)
    {
        ALuint buffer;
        alCall(alSourceUnqueueBuffers, audioData.source, 1, &buffer);

        char *data = new char[BUFFER_SIZE];
        std::memset(data, 0, BUFFER_SIZE);

        ALsizei dataSizeToBuffer = 0;
        std::int32_t sizeRead = 0;

        while (sizeRead < BUFFER_SIZE)
        {
            std::int32_t result = ov_read(&audioData.oggVorbisFile, &data[sizeRead], BUFFER_SIZE - sizeRead, 0, 2, 1, &audioData.oggCurrentSection);
            if (result == OV_HOLE)
            {
                std::cerr << "ERROR: OV_HOLE found in update of buffer " << std::endl;
                break;
            }
            else if (result == OV_EBADLINK)
            {
                std::cerr << "ERROR: OV_EBADLINK found in update of buffer " << std::endl;
                break;
            }
            else if (result == OV_EINVAL)
            {
                std::cerr << "ERROR: OV_EINVAL found in update of buffer " << std::endl;
                break;
            }
            else if (result == 0)
            {
                std::int32_t seekResult = ov_raw_seek(&audioData.oggVorbisFile, 0);
                if (seekResult == OV_ENOSEEK)
                    std::cerr << "ERROR: OV_ENOSEEK found when trying to loop" << std::endl;
                else if (seekResult == OV_EINVAL)
                    std::cerr << "ERROR: OV_EINVAL found when trying to loop" << std::endl;
                else if (seekResult == OV_EREAD)
                    std::cerr << "ERROR: OV_EREAD found when trying to loop" << std::endl;
                else if (seekResult == OV_EFAULT)
                    std::cerr << "ERROR: OV_EFAULT found when trying to loop" << std::endl;
                else if (seekResult == OV_EOF)
                    std::cerr << "ERROR: OV_EOF found when trying to loop" << std::endl;
                else if (seekResult == OV_EBADLINK)
                    std::cerr << "ERROR: OV_EBADLINK found when trying to loop" << std::endl;

                if (seekResult != 0)
                {
                    std::cerr << "ERROR: Unknown error in ov_raw_seek" << std::endl;
                    return;
                }
            }
            sizeRead += result;
        }
        dataSizeToBuffer = sizeRead;

        if (dataSizeToBuffer > 0)
        {
            alCall(alBufferData, buffer, audioData.format, data, dataSizeToBuffer, audioData.sampleRate);
            alCall(alSourceQueueBuffers, audioData.source, 1, &buffer);
        }

        if (dataSizeToBuffer < BUFFER_SIZE)
        {
            std::cout << "Data missing" << std::endl;
        }

        ALint state;
        alCall(alGetSourcei, audioData.source, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING)
        {
            alCall(alSourceStop, audioData.source);
            alCall(alSourcePlay, audioData.source);
        }

        delete[] data;
    }
}
void stop_stream(const StreamingAudioData &audioData)
{
    alCall(alSourceStop, audioData.source);
}
/*
https://indiegamedev.net/2020/02/15/the-complete-guide-to-openal-with-c-part-1-playing-a-sound/
*/
int main()
{
    ALCdevice *device = alcOpenDevice(NULL); // select the "preferred device"
    if (!device)
    {
        std::cerr << "open device error" << std::endl;
        return -1;
    }
    ALCcontext *acctx = alcCreateContext(device, NULL);
    alcMakeContextCurrent(acctx);
    StreamingAudioData data;
    if (!create_stream_from_file("./nice_music.ogg", data))
    {
        std::cerr << "error" << std::endl;
        return -1;
    }
    play_stream(data);
    while (true)
    {
        // int res = std::cin.get();
        // if ('q' == res)
        //     break;
        update_stream(data); // TODO
    }

    alcDestroyContext(acctx);
    alcCloseDevice(device);
    return 0;
}