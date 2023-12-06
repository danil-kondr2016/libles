#include <les/version.h>

#define LES_MAJOR_VERSION 0
#define LES_MINOR_VERSION 1
#define LES_PATCH_VERSION 3

#define XSTR(x) #x
#define STR(x) XSTR(x)

#define LES_VERSION \
	    STR(LES_MAJOR_VERSION) \
	"." STR(LES_MINOR_VERSION) \
	"." STR(LES_PATCH_VERSION)

static const char __version[] = LES_VERSION;

LIBLES_API
const char *les_version()
{
	return __version;
}
