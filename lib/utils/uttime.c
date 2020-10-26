#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

long get_current_millis() {
	struct timeval t;
	gettimeofday(&t, NULL);
	ulong s1 = t.tv_sec * 1000;
	ulong s2 = t.tv_usec / 1000;
	return s1 + s2;
}


char* format_timestamp(long millis) {
    long sec = millis / 1000;
    struct tm *ptm = localtime(&sec);
    
    if (ptm == NULL) {
        perror("localtime");
        return NULL;
    }
    
    char formatted[16]; 
    snprintf(formatted, 16, "%04d%02d%02d_%02d%02d%02d", 1900 + ptm->tm_year, ptm->tm_mon + 1, ptm->tm_mday, 
        ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    return strndup(formatted, strlen(formatted));
}


char* get_formatted_datetime() {
	long rawtime = get_current_millis();
    
    if (rawtime == -1) {
        perror("time");
        return NULL;
    }
    
    return format_timestamp(rawtime);
}
