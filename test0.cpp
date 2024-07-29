#include <iostream>
#include <cstdio>
#include <chrono>
#include <thread>
#include "dbg.hpp"
void tryread(std::istream &in, std::ostream &out)
{
    if (in.rdbuf()->in_avail() > 0)
    {
        out << "cin read some message" << std::endl;
        std::string line;
        std::getline(in, line);
        using std::operator""s;
        if ("pause"s == line)
        {
            out << "input pause" << std::endl;
        }
    }
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

int main1()
{
    auto pbuf = std::cin.rdbuf();
    void *ptr = pbuf;

    std::cout << ptr << std::endl;
    while (true)
    {
        const auto f = []() -> bool
        {
            return std::cin.rdbuf()->in_avail() > 0;
        };
        if (f())
        {
            const std::chrono::time_point n = std::chrono::system_clock::now();
            const auto nms = std::chrono::time_point_cast<std::chrono::milliseconds>(n);
            std::cout << nms.time_since_epoch().count() << std::endl;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    return 0;
}

static int strContains()
{
    // find first charactor match and length of remain charactor left more than equals right
    // if false then next charactor
    // if true then compare the remain charactor
    //      if true then return index of matched first charactor
    //      if false then next charactor
    return 0;
}

int str_contains(const char *long_str, const char *short_str, int max)
{
    int i, j;
    size_t ll = strnlen(long_str, max);
    size_t sl = strnlen(short_str, max);
    const int slm1 = sl - 1;
    const int slm2 = sl - 2;
    if (ll == max || sl == max)
        return -1;
    if (sl > ll)
        return -1;
    for (i = 0; i < ll - sl; i++)
    {
        if (long_str[i] == short_str[0])
        {
            for (j = 1; j < slm1; j++)
            {
                if (long_str[i + j] != short_str[j])
                {
                    break;
                }
                else
                {
                    if (slm2 == j)
                    {
                        if ('\0' == short_str[j])
                        {
                            return i;
                        }
                    }
                    else if (j < slm2)
                    {
                        continue;
                    }
                }
            }
        }
    }
    return -1;
}
int testcast(){
    {
        int var = 0;
        void *ptr = reinterpret_cast<void *>(var);
        ptrdiff_t diff = reinterpret_cast<ptrdiff_t>(ptr);
        // void *ptr2 = static_cast<void *>(var); //error
    }
    {
        int i = 1;
        const int *pi= &i;
        int* np = const_cast<int*>(pi);
        (*np)++;
        std::cout << i << std::endl;
    }    
    return 0;
}

int main(){
    const int SRATE = 44100;
    const int SSIZE = 1024;
    alc_print_devices();
    ALCdevice *device = alcCaptureOpenDevice(NULL, SRATE, AL_FORMAT_STEREO16, SSIZE);
    alcCaptureCloseDevice(device);
    return 0;
}