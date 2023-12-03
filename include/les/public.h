#pragma once
#ifndef _LES_PUBLIC_H_
#define _LES_PUBLIC_H_

#if defined(_LES_DLL)
# if defined(_WIN32)||defined(__WIN32__)||defined(WIN32)
#  define LIBLES_API __declspec(dllexport)
# elif defined(__ELF__)
#  define LIBLES_API __attribute__((visibility("default")))
# else
#  define LIBLES_API
# endif
#else
# if defined(_WIN32)
#  define LIBLES_API __declspec(dllimport)
# else
#  define LIBLES_API
# endif
#endif

#ifdef __cplusplus
# define LES_DEF_BEGIN extern "C" {
# define LES_DEF_END }
#else
# define LES_DEF_BEGIN 
# define LES_DEF_END
#endif

#endif
