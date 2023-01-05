#ifndef INC_COLS_H
#define INC_COLS_H
#include <string>
#include <vector>
/// Class for breaking a string up into an array of strings using given separators.
class Cols {
  public:
    Cols();
    /// Separate the given line according to given separators.
    int Split(std::string const&, const char*);
    /// \return Specified column
    std::string const& operator[](unsigned int i) const { return columns_[i]; }
    /// \return current number of columns
    unsigned int Ncolumns() const { return columns_.size(); }
    /// \return Unmarked Column next to specified unmarked key.
    std::string GetKey(std::string const&);
    /// \return Next unmarked column.
    std::string NextColumn();

    typedef std::vector<std::string>::const_iterator const_iterator;
    const_iterator begin() const { return columns_.begin(); }
    const_iterator end()   const { return columns_.end(); }
  private:
    typedef std::vector<std::string> Sarray;
    Sarray columns_;
    typedef std::vector<bool> Barray;
    Barray marked_;
};
#endif
