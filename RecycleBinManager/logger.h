#pragma once

// Logging
#ifndef NDEBUG
#define LOG(format, ...) do             \
    {                                   \
        wchar_t temp[1024];             \
        _snwprintf(temp,                \
                   ARRAYSIZE(temp),     \
                   format,              \
                   __VA_ARGS__);        \
        temp[1023] = 0;                 \
        OutputDebugStringW(temp);       \
    } while (0)
#else
#define LOG(format, ...) (void) (format)
#endif