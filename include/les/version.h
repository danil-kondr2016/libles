#pragma once
#ifndef _LES_VERSION_H_
#define _LES_VERSION_H_

#include <les/public.h>

#define LES_MAJOR_VERSION 0
#define LES_MINOR_VERSION 1
#define LES_PATCH_VERSION 1

#define XSTR(x) #x
#define STR(x) XSTR(x)

#define LES_VERSION \
	    STR(LES_MAJOR_VERSION) \
	"." STR(LES_MINOR_VERSION) \
	"." STR(LES_PATCH_VERSION)

LIBLES_API
const char *les_version();

#endif