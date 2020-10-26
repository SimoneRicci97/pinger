#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "string_utils.h"

char* pattern_substring(const char* s, char* begin_pattern, char* end_pattern) {
	if(!s) return NULL;
	char* copy = strndup(s, strlen(s));
	char* index = strstr(copy, begin_pattern);
	char* index1 = strstr(copy, end_pattern);
	if(!index || !index1) {
		free(copy);
		return NULL;
	}
	index += strlen(begin_pattern)  * sizeof(char);
	long sublen = (index1 - index) / sizeof(char);
	printf("%s -> (%s|%s) %ld\n", copy, index, index1, sublen);
	if(sublen <= 0) {
		free(copy);
		return NULL;
	}
	char* s1 = strndup(index, sublen);
	free(copy);
	return s1;
}


char* string_concat(char* s, char* s1) {
	char* ss[2];

	ss[0] = s;
	ss[1] = s1;

	return string_builder(ss, 2, NULL);
}

char* string_builder(char** ss, int len, char* sep) {
	char* s;
	if(sep) {
		s = malloc((sum_lens(ss, len) + (len * strlen(sep) - 1)) * sizeof(char) + sizeof(char));
	} else {
		s = malloc(sum_lens(ss, len) * sizeof(char) + sizeof(char));
	}
	strncpy(s, ss[0], strlen(ss[0]) + 1);
	for(int i=1; i<len; i++) {
		strncat(s, ss[i], strlen(ss[i]));
		if(sep) {
			strncat(s, sep, strlen(sep));  
		}
	}
	return s;
}


int tkncnts(const char* s, char* t) {
	char* s1 = strndup(s, strlen(s));
	char* tmp = s1;
	int count = 0;
	char* ptr = strstr(s1, t);
	while(ptr != NULL) {
		count ++;
		s1 = ptr + strlen(t) * sizeof(char);
		ptr = strstr(s1, t);
	}
	free(tmp);
	return count;
}



int tkncntc(const char* s, char t) {
	char* s1 = strndup(s, strlen(s));
	char* tmp = s1;
	int count = 0;
	char* ptr = strchr(s1, t);
	while(ptr != NULL) {
		count ++;
		s1 = ptr + sizeof(char);
		ptr = strchr(s1, t);
	}
	free(tmp);
	return count;
}


int sum_lens(char** s, int len) {
	int sum = 0;
	int i = 0;
	while(i < len) {
		sum += strlen(s[i]);
		i++;
	}
	return sum;
}


char** strspltc(const char* s, char sep, int* final_size) {
	char* s1 = strndup(s, strlen(s));
	char* tmp = s1;
	int nsep = tkncntc(s1, sep);
	char** res = malloc((nsep + 1) * sizeof(char*));
	char* ptr = strchr(s1, sep);
	int i = 0;
	while(ptr != NULL) {
		int tknlen = (ptr - s1) * sizeof(char);
		res[i] = strndup(s1, tknlen);
		s1 = ptr + sizeof(char);
		ptr = strchr(s1, sep);
		i++;
	}
	res[i] = strndup(s1, strlen(s1));
	free(tmp);
	*final_size = nsep + 1;
	return res;
}


char** strsplts(const char* s, char* sep, int* final_size) {
	char* s1 = strndup(s, strlen(s));
	char* tmp = s1;
	int nsep = tkncnts(s1, sep);
	char** res = malloc((nsep + 1) * sizeof(char*));
	char* ptr = strstr(s1, sep);
	int i = 0;
	while(ptr != NULL) {
		int tknlen = (ptr - s1 + sizeof(char)) * sizeof(char);
		res[i] = strndup(s1, tknlen);
		s1 = ptr + strlen(sep) * sizeof(char);
		ptr = strstr(s1, sep);
		i++;
	}
	res[i] = strndup(s1, strlen(s1));
	free(tmp);
	*final_size = nsep + 1;
	return res;
}