#ifndef __UTTIME__
#define __UTTIME__

typedef unsigned long ulong;

long get_current_millis();

/**
 * return now date time in format yyyyMMdd_HHmmss
 */
char* get_formatted_datetime();

/**
 * return date time in format yyyyMMdd_HHmmss
 */
char* format_timestamp(long ts);

#endif
