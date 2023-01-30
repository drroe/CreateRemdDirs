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

/** \return True if user selects y. */
bool Messages::YesNoPrompt(const char* msg) {
  char buffer[128];
  int selection = 0;
  while (selection == 0) {
    Msg("%s [y|n]> ");
    char* ptr = fgets( buffer, 127, stdin );
    if (ptr != 0) {
      if (buffer[0] == 'y') {
        selection = 1;
        break;
      } else if (buffer[0] == 'n') {
        selection = -1;
        break;
      }
    }
  }
  if (selection == 1) return true;
  return false;
}
