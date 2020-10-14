#ifndef __PSCHECK__
#define __PSCHECK__

#include "ping_list.h"

void check_stats(chunk_list* chunks, ping_time_chunk* ping);

void report(chunk_list* chunks, char* out_range_stats, ping_time_chunk* ping);

#endif