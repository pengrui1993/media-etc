#include <AL/al.h>
#include <AL/alc.h>
#include <cinttypes>
#include <string>
#include <iostream>
#include <cstdlib>
#include <string>
#define DEBUG 1
#if (DEBUG)
#define alCall(function, ...) alCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)
#define alcCall(function, ...) alcCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)
#else
#define alCall(function, ...) function(__VA_ARGS__)
#define alcCall(function, ...) function(__VA_ARGS__)
#endif

using ALPath = std::string;
using ALLine = const std::uint_fast32_t;
void alc_print_devices();
void clear_alc_errors();
void clear_al_errors();
void check_alc_errors(ALPath filename, ALLine line,ALCdevice* device);
void check_al_errors(ALPath filename, ALLine line);
template <typename alFunction, typename... Params>
auto alCallImpl(ALPath filename, ALLine line, alFunction function, Params... params)
    -> typename std::enable_if<std::is_same<void, decltype(function(params...))>::value, decltype(function(params...))>::type
{
    function(std::forward<Params>(params)...);
    check_al_errors(filename, line);
}

template <typename alFunction, typename... Params>
auto alCallImpl(ALPath filename, ALLine line, alFunction function, Params... params)
    -> typename std::enable_if<!std::is_same<void, decltype(function(params...))>::value, decltype(function(params...))>::type
{
    auto ret = function(std::forward<Params>(params)...);
    check_al_errors(filename, line);
    return ret;
}

template <typename alFunction, typename... Params>
auto alcCallImpl(ALPath filename, ALLine line, alFunction function,ALCdevice*d, Params... params)
    -> typename std::enable_if<std::is_same<void, decltype(function(d,params...))>::value, decltype(function(d,params...))>::type
{
    function(d,std::forward<Params>(params)...);
    check_alc_errors(filename, line,d);
}
template <typename alFunction, typename... Params>
auto alcCallImpl(ALPath filename, ALLine line, alFunction function,ALCdevice*d, Params... params)
    -> typename std::enable_if<!std::is_same<void, decltype(function(d,params...))>::value, decltype(function(d,params...))>::type
{
    auto ret = function(d,std::forward<Params>(params)...);
    check_alc_errors(filename, line,d);
    return ret;
}


