#include <cstdio>
#include <cstdarg>

void ErrorMsg(const char* format, ...) {
  fprintf(stderr,"Error: ");
  va_list args;
  va_start(args, format);
  vfprintf(stderr,format,args);
  va_end(args);
}

void Msg(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stdout,format,args);
  va_end(args);
}
