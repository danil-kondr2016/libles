#include <les/version.h>

static const char __version[] = LES_VERSION;

const char *les_version()
{
	return __version;
}
