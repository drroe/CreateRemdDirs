#ifndef INC_FILENAMEARRAY_H
#define INC_FILENAMEARRAY_H
#include <vector>
#include <string>
/// Class used to generate either a single file name or an array of similarly-named files
class FileNameArray {
  public:
    /// CONSTRUCTOR - take base name/path, start run #, current run #, previous dir name, file extension, file ext. min width
    FileNameArray(std::string const&, int, int, std::string const&, std::string const&, int);
    /// CONSTRUCTOR - always use base name/path - take base, file extension, file ext. min width
    FileNameArray(std::string const&, std::string const&, int);
    /// Generate specified number of file name(s) with optional start coords
    int Generate(unsigned int, std::string const&);
    /// \return true if no file names
    bool empty() const { return files_.empty(); }
    /// \return file name at index
    std::string const& operator[](int idx) const { return files_[idx]; }
  private:
    typedef std::vector<std::string> Sarray;
    /// \return single file
    Sarray single_file(std::string const&, std::string const&) const;
    /// \return multiple files
    Sarray multi_file(std::string const&, std::string const&, unsigned int) const;
    /// \return Numerical extension of width needed to hold given max
    std::string NumericalExt(int, int) const;

    Sarray files_; ///< Hold all generated file names
    std::string base_; ///< Base initial file name/dir name (runNum == 0)
    int startRunNum_;  ///< Starting run number
    int runNum_;       ///< Current run number
    std::string prevDir_; ///< Previous run directory name
    std::string crd_ext_; ///< Coordinates extension (for multiple files)
    int fileExtWidth_;    ///< Min file numerical extension width
};
#endif
