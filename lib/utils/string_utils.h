#ifndef __STRING_UTILS__
#define __STRING_UTILS__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char* pattern_substring(const char* s, char* begin_pattern, char* end_pattern);


char* string_concat(char* s, char* s1);


int tkncntc(const char* s, char t);


int tkncnts(const char* s, char* t);


int sum_lens(char** s, int len);


char* string_builder(char** ss, int len, char* sep);


char** strspltc(const char* s, char sep, int* final_size);


char** strsplts(const char* s, char* sep, int* final_size);

#endif
