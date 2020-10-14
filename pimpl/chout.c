#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "chout.h"
#include "string_utils.h"

#define CHOUT_BUFFER_SIZE 128

char** format_child_output(char* s);

char** format_child_output(char* s) {
	char* store = s;
	char** lines;
	int n_lines = tkncntc(s, '\n');
	lines = malloc((n_lines + 1) * sizeof(char*));
	
	int i = 0;
	size_t index = strcspn(s, "\n");
	while(s[index] != '\0' && strlen(s) > 0 && s[index] == '\n') {
		lines[i] = malloc((index + 1) * sizeof(char));
		strncpy(lines[i], s, index);
		lines[i][index] = '\0';
		// DEBUG
		//printf("%d: %s\n", i, lines[i]);
		
		i++;
		if(s[index] != '\0') {
			s = &s[index+1];
			index = strcspn(s, "\n");
		}
	}
	lines[i] = NULL;
	free(store);
	return lines;
}

char** read_child_output(int fd) {
	char buffer[CHOUT_BUFFER_SIZE + 1];
	char** out = malloc(128 * sizeof(char*));
	int count = 0;
	ssize_t r = read(fd, buffer, CHOUT_BUFFER_SIZE);
	while(r != 0) {
		buffer[r] = '\0';	
		if(r == -1 && errno != EINTR) {
			printf("errno: %d\nEINTR: %d\n", errno, EINTR);
			perror("read");
			exit(1);
		} else {
			out[count] = strndup(buffer, strlen(buffer));
			count++;
		}
		r = read(fd, buffer, CHOUT_BUFFER_SIZE);
	}
	out[count] = NULL;
	close(fd);
	char** formatted = format_child_output(string_builder(out, count, NULL));
	for(int i=0; i<count; i++) {
		free(out[i]);
	}
	free(out);
	return formatted;
}