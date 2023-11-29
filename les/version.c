#define _LES_DLL
#include <les/version.h>

static const char __version[] = LES_VERSION;

LIBLES_API
const char *les_version()
{
	return __version;
}
