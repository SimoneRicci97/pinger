#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "string_utils.h"

char* pattern_substring(const char* s, char* begin_pattern, char* end_pattern) {
	char* copy = strndup(s, strlen(s));
	char* index = strstr(copy, begin_pattern);
	char* index1 = strstr(copy, end_pattern);
	index += strlen(begin_pattern)  * sizeof(char);
	char* s1 = strndup(index, (index1 - index) / sizeof(char));
	free(copy);
	return s1;
}


char* string_concat(char* s, char* s1) {
	char** ss = malloc(2 * sizeof(char*));

	ss[0] = s;
	ss[1] = s1;

	return string_builder(ss, 2, NULL);
}

char* string_builder(char** ss, int len, char* sep) {
	char* s;
	if(sep) {
		s = malloc((sum_lens(ss, len) + (len * strlen(sep) - 1)) * sizeof(char) + sizeof(char));
	} else {
		s = malloc(sum_lens(ss, len) * sizeof(char));
	}
	int index = 0;
	for(int i=0; i<len; i++) {
		strncpy(&s[index], ss[i], strlen(ss[i]));
		index += strlen(ss[i]);
		if(sep) {
			strncpy(&s[index], sep, strlen(sep));  
			index += strlen(sep);
		}
	}
	if(sep) s[index-1] = '\0';
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
	res[i] = strndup(s1, strlen(s));
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