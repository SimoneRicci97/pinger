#ifndef __PINGER_ARCHIVE__
#define __PINGER_ARCHIVE__

#define TAR "/bin/tar"
#define ZIP "/usr/bin/zip"


typedef struct _archive_args {
	char dir[256];
	char archive_sh[256];
} archive_args_t;

void archive(void* archive_args);

void reset(void* hosts);


#endif
