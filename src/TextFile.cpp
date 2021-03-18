#include <cstdio>
#include <cstdarg>
#include <cstring>
#include "TextFile.h"
#include "Messages.h"

using namespace Messages;

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

int TextFile::OpenPipe(std::string const& cmd) {
  FILE* pipe = popen(cmd.c_str(), "r");
  if (pipe == 0) {
    ErrorMsg("Opening pipe: '%s'\n", cmd.c_str());
    return 1;
  }
  file_ = (void*)pipe;
  isPipe_ = true;
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
  if (file_ != 0) {
    if (isPipe_) {
      pclose((FILE*)file_);
      isPipe_ = false;
    } else
      fclose((FILE*)file_);
  }
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

TextFile::OptArray TextFile::GetOptionsArray(std::string const& fname, int debug) {
  OptArray options;
  if (OpenRead( fname )) return options;
  const char* SEP = " \t\n";
  int ncols = GetColumns( SEP );
  while (ncols > -1) {
    if (ncols > 0 && tokens_[0][0] != '#') {
      if (ncols < 2) {
        ErrorMsg("Malformed input: %s\n", buffer_);
        options.clear();
        break;
      }
      std::string OPT = tokens_[0];
      std::string VAR = tokens_[1];
      for (int i = 2; i < ncols; i++)
        VAR += (" " + tokens_[i]);
      if (debug > 0)
        Msg("    File '%s': Option: %s  Variable: %s\n", fname.c_str(), OPT.c_str(), VAR.c_str());
      options.push_back( Spair(OPT, VAR) );
    }
    ncols = GetColumns( SEP );
  }
  Close();
  return options;
}
