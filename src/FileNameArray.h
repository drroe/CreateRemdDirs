#ifndef INC_FILENAMEARRAY_H
#define INC_FILENAMEARRAY_H
#include <vector>
#include <string>
/// Class used to generate either a single file name or an array of similarly-named files
class FileNameArray {
  public:
    /// Indicate whether previous will be dir or file
    enum PrevType { IS_DIR = 0, IS_FILE };
    /// CONSTRUCTOR - take base name/path, previous dir/coords name, previous type, file extension, file ext. min width
    FileNameArray(std::string const&, std::string const&, PrevType, std::string const&, int);
    /// Generate specified number of file name(s) for initial (true) or subsequent (false) run
    int Generate(unsigned int, bool);
    /// \return true if no file names
    bool empty() const { return files_.empty(); }
    /// \return file name at index
    std::string const& operator[](int idx) const { return files_[idx]; }
  private:
    typedef std::vector<std::string> Sarray;
    /// \return multiple files
    Sarray multi_file(std::string const&, unsigned int, PrevType) const;
    /// \return Numerical extension of width needed to hold given max
    std::string NumericalExt(int, int) const;

    Sarray files_; ///< Hold all generated file names
    std::string base_; ///< Base initial file name/dir name (runNum == 0)
    std::string prevDir_; ///< Previous run directory/file name
    PrevType prevType_;   ///< Previous run type (directory/file)
    std::string crd_ext_; ///< Coordinates extension (for multiple files)
    int fileExtWidth_;    ///< Min file numerical extension width
};
#endif
