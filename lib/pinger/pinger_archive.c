#include "pinger_archive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uttime.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <htable.h>
#include <ping_list.h>

#include <pthread.h>

#define MV "/bin/mv"
#define USER_SH "./pinger_archive.sh"

void _tar(char* dir);
void _zip(char* dir);
void _user_archive(char* dir);
void _mvclear(char* dir);

void archive(void* archive_args_v) {
	archive_args_t* archive_args = (archive_args_t*) archive_args_v;
	int p = fork();
	if(p > 0) {
		int status;
		waitpid(p, &status, 0);
	} else if(p == 0) {
		_user_archive(archive_args->dir);
	} else {
		perror("Running archive tool");
	}
}


void reset(void* hosts_v) {
	htable* hosts = (htable*) hosts_v;
	pthread_mutex_lock(&hosts->mutex);
	for(int i=0; i<hosts->size; i++) {
		if(hosts->keys[i]) {
			char* key = (char*) hosts->keys[i]->head->key;
			chunk_list* cl = hosts->get(hosts, key, strlen(key));
			cl->clear(cl);
			fprintf(stderr, "Now size is %d\n", ((chunk_list*) hosts->get(hosts, key, strlen(key)))->size);
		}
	}
	pthread_mutex_unlock(&hosts->mutex);
}


void _user_archive(char* dir) {
	char* ua_args[3] = {USER_SH, get_formatted_datetime(), NULL};
	execv(USER_SH, ua_args);
}


void _mvclear(char* dir) {
	char source[128], dest[128];
	snprintf(source, 128, "%s/*.log", dir);
	snprintf(dest, 128, "%s/archived/", dir);
	char* mvargs[4] = {MV, source, dest, NULL};
	int p = fork();
	if(p == 0) {
		execv(MV, mvargs);
	} else if(p > 0) {
		int status;
		waitpid(p, &status, 0);
	} else {
		perror("Cannot move files");
	}
}

void _tar(char* dir) {
	char archive_name[128];
	char archive_content[128];
	sprintf(archive_name, "%s/pinger.archive.%s.log.tar", dir, get_formatted_datetime());
	sprintf(archive_content, "%s/*.log", dir);
	char* tar_args[6] = {TAR, "-cf", archive_name, archive_content, NULL};
	execv(TAR, tar_args);
}

void _zip(char* dir) {
	char archive_name[128];
	char archive_content[128];
	sprintf(archive_name, "%s/pinger.archive.%s.log.zip", dir, get_formatted_datetime());
	sprintf(archive_content, "%s/*.log", dir);
	char* tar_args[5] = {ZIP, archive_name, archive_content, NULL};
	execv(ZIP, tar_args);
}
