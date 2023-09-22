#ifndef __AUDIO_ENGINE_H_
#define __AUDIO_ENGINE_H_
#include <cinttypes>
#include <functional>
struct AudioConfig;
enum class AudioType : uint8_t
{
    OOPS,
    CLIP,
    MUSIC
};
struct Audio
{
    AudioType type;
    void *ptr;
    bool isValid() { return type != AudioType::OOPS; }
};
struct AudioResource
{
};
class AudioEngine
{
protected:
    AudioEngine();
    virtual ~AudioEngine();
    AudioEngine(const AudioEngine &) = delete;
    AudioEngine(AudioEngine &&) = delete;
    AudioEngine &operator=(const AudioEngine &) = delete;
    AudioEngine &operator=(AudioEngine &&) = delete;
    AudioEngine *operator&() = delete;
    const AudioEngine *operator&() const = delete;

public:
    typedef std::function<void(void)> Callback;
    static AudioConfig config(AudioConfig &);
    static AudioEngine &getInstance();
    static const Callback nullcb;
    typedef struct
    {
        Callback loaded;
        Callback startcb;
        Callback updatecb;
        Callback finishcb;
    } Callbacks;
    static const Callbacks nullcbs;

public:
    Audio play(const AudioResource &, bool repeat = false, Callbacks cbs = nullcbs);
    bool pause();
    bool resume();
    bool stop();
    bool pause(const Audio &);
    bool resume(const Audio &);
    bool stop(const Audio &);

private:
};

const auto AudioEngine::nullcb{[]() {}};
const auto AudioEngine::nullcbs{nullcb, nullcb, nullcb, nullcb};
#endif