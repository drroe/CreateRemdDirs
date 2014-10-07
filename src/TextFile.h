#ifndef INC_TEXTFILE_H
#define INC_TEXTFILE_H
#include <string>
/// Simple wrapper for text file.
class TextFile {
  public:
    TextFile() : file_(0) {}
    ~TextFile();
    int OpenRead(std::string const&);
    int OpenWrite(std::string const&);
    void Close();
    // NOTE: Return char* so strtok can work
    char* Gets();
    // \return next line as string, no newline.
    std::string GetString();
    int Printf(const char*, ...);
  private:
    static const unsigned int BUF_SIZE = 1024;
    char buffer_[BUF_SIZE];
    void* file_;
};
#endif
