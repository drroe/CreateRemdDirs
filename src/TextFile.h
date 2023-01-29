#ifndef INC_TEXTFILE_H
#define INC_TEXTFILE_H
#include <string>
#include "OptArray.h"
/// Simple wrapper for text file.
class TextFile {
  public:
    /// CONSTRUCTOR
    TextFile() : file_(0), isPipe_(false) {}
    ~TextFile();
    int OpenRead(std::string const&);
    int OpenPipe(std::string const&);
    int OpenWrite(std::string const&);
    void Close();
    /// \return next line in internal char buffer 
    const char* Gets();
    /// \return next line as string, no newline.
    std::string GetString();
    /// \return Number of columns in next line; text stored in tokens_. // FIXME deprecate for Cols?
    int GetColumns(const char*);
    /// \return text in specified column
    std::string const& Token(int i) const { return tokens_[i]; }
    /// Print formatted text to file.
    int Printf(const char*, ...);
    /// \return pointer to internal buffer.
    const char* Buffer() const { return buffer_; }
    /// \return Options array from <OPT> <VAR> style file.
    OptArray GetOptionsArray(std::string const&, int);
  private:
    static const unsigned int BUF_SIZE = 8192;
    char buffer_[BUF_SIZE];
    void* file_;
    typedef std::vector<std::string> Sarray;
    Sarray tokens_;
    bool isPipe_;
};
#endif
