#ifndef INC_MDPACKAGE_H
#define INC_MDPACKAGE_H
#include <string>
class RepIndexArray;
class MdOptions;
/// Abstract base class for various MD packages.
class MdPackage {
  public:
    /// CONSTRUCTOR
    MdPackage() : debug_(0) {}
    /// COPY CONSTRUCTOR
    MdPackage(MdPackage const& rhs) : debug_(rhs.debug_) {}
    /// ASSIGNMENT
    MdPackage& operator=(MdPackage const& rhs) {
      if (&rhs == this) return *this;
      debug_ = rhs.debug_;
      return *this;
    }
    /// DESTRUCTOR - virtual since inherited
    virtual ~MdPackage() {}
    /// \return a Copy of the class
    virtual MdPackage* Copy() const = 0;
    /// Read package-specific input options from a file
    virtual int ReadInputOptions(std::string const&) = 0;
    /// Write package-specific input file
    virtual int WriteMdInputFile(MdOptions const&,std::string const&, int, std::string const&,
                                 RepIndexArray const&, unsigned int) const = 0;

    void SetDebug(int d) { debug_ = d; }
    /// \return debug level
    int Debug() const { return debug_; }
  private:
    int debug_;
};
#endif
