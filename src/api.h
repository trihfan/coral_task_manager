#pragma once

#if defined(WIN32)
    #ifdef API_EXPORT
        #define api __declspec(dllexport)
    #else
        #define api __declspec(dllimport)
    #endif
#endif
