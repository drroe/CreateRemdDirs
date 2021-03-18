#include "Messages.h"
#include <cstdio>
#include <cstdarg>

void Messages::ErrorMsg(const char* format, ...) {
  fprintf(stderr,"Error: ");
  va_list args;
  va_start(args, format);
  vfprintf(stderr,format,args);
  va_end(args);
}

void Messages::Msg(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stdout,format,args);
  va_end(args);
}
