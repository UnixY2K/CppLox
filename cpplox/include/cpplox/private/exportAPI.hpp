#pragma once

#if defined(LIB_CppLoxLIBRARY_EXPORT)
#   define LIB_CppLoxAPI EXPORT
#else
#   define LIB_CppLoxAPI IMPORT
#endif

#if defined(_MSC_VER)
    //  Microsoft 
    #define EXPORT __declspec(dllexport)
    #define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    //  GCC
    #define EXPORT __attribute__((visibility("default")))
    #define IMPORT
#else
    //  do nothing
    #define EXPORT
    #define IMPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif