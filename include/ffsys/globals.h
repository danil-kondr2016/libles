/** ffsys: define global symbols */

#include <ffsys/base.h>
int _ffcpu_features;

#ifdef FF_WIN

#include <ffsys/error.h>
char _fferr_buffer[1024];

#else // UNIX:

#include <ffsys/environ.h>
char **_ff_environ;

#endif
