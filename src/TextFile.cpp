#include <cstdio>
#include <cstdarg>
#include <cstring>
#include "TextFile.h"
#include "Messages.h"

TextFile::~TextFile() { Close(); }

int TextFile::OpenRead(std::string const& fname) {
  FILE* infile = fopen(fname.c_str(), "rb");
  if (infile == 0) {
    ErrorMsg("Opening file '%s'\n", fname.c_str());
    return 1;
  }
  file_ = (void*)infile;
  return 0;
}

int TextFile::OpenWrite(std::string const& fname) {
  FILE* outfile = fopen(fname.c_str(), "wb");
  if (outfile == 0) {
    ErrorMsg("Opening file '%s'\n", fname.c_str());
    return 1;
  }
  file_ = (void*)outfile;
  return 0;
}

void TextFile::Close() {
  if (file_ != 0) fclose((FILE*)file_);
  file_ = 0;
}

const char* TextFile::Gets() {
  if (file_ == 0) return 0;
  return (const char*)fgets(buffer_, BUF_SIZE-1, (FILE*)file_);
}

std::string TextFile::GetString() {
  if (file_ == 0) return std::string("");
  char* ptr = fgets(buffer_, BUF_SIZE-1, (FILE*)file_);
  if (ptr == 0) return std::string("");
  // Remove any newline.
  for (unsigned int i = 0; i != BUF_SIZE; i++) {
    if (buffer_[i] == '\0') break;
    if (buffer_[i] == '\n') {
      buffer_[i] = '\0';
      break;
    }
  }
  return std::string(buffer_);
}

int TextFile::GetColumns( const char* SEP ) {
  if (file_ == 0) return -1;
  char* ptr = fgets(buffer_, BUF_SIZE-1, (FILE*)file_);
  if (ptr == 0) return -1;
  tokens_.clear();
  ptr = strtok(buffer_, SEP);
  while (ptr != 0) {
    tokens_.push_back( std::string(ptr) );
    ptr = strtok(0, SEP);
  }
  return (int)tokens_.size();
}

int TextFile::Printf(const char *format, ...) {
  if (file_==0) return 1;
  va_list args;
  va_start(args, format);
  vsprintf(buffer_,format,args);
  fwrite(buffer_, 1, strlen(buffer_), (FILE*)file_);
  va_end(args);
  return 0;
}
