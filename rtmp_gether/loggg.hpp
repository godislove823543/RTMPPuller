//
//  loggg.hpp
//  practice_stream
//
//  Created by scarlett on 2023/10/17.
//

#ifndef loggg_hpp
#define loggg_hpp

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>


void log(const char *tag, int line, const char *format,...)
{
    va_list args;
    timeval tv;
    gettimeofday(&tv,NULL);
    
    time_t time_seconds = time(0);
    tm now_time;
    localtime_r(&time_seconds, &now_time);
    
    char buff[100];
    snprintf(buff,sizeof(buff),"%d-%d-%d %02d:%02d:%02d [%s][%d]: %s\n", now_time.tm_year + 1900, now_time.tm_mon+1,now_time.tm_mday,now_time.tm_hour, now_time.tm_min, now_time.tm_sec,tag,line,format);
    va_start(args, format);
    vprintf(buff, args);
    va_end(args);
    
}
#endif /* loggg_hpp */
