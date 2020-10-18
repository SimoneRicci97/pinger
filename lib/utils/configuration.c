#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "htable.h"
#include "string_utils.h"

#define CONF_BUFFER_SIZE 512

void _destroy_string(void* v);

htable* load_configuration(char* conf_file) {
	FILE* f = fopen(conf_file, "r");
	char buffer[CONF_BUFFER_SIZE];
	htable* conf = new_htable(_destroy_string, _destroy_string);
	char* b = fgets(buffer, CONF_BUFFER_SIZE, f);
	while(b != NULL) {
		if(buffer[0] != '#' && buffer[0] != '\n' && strlen(buffer) > 0) {
			int len;
			char** configuration = strsplts(buffer, " = ", &len);
			char* name = strndup(configuration[0], strlen(configuration[0]) - 1);
			char* value = strndup(configuration[1], strlen(configuration[1]) - 1);
			free(configuration[0]);
			free(configuration[1]);
			free(configuration);
			conf->put(conf, name, value, strlen(name));
		} 
		b = fgets(buffer, CONF_BUFFER_SIZE, f);
	}
	fclose(f);
	return conf;
}


void _destroy_string(void* v) {
	free((char*) v);
}
