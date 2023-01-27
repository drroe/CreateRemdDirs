#ifndef INC_MDPACKAGE_H
#define INC_MDPACKAGE_H
#include <string>
#include <vector>
class RepIndexArray;
class MdOptions;
class Creator;
class RunStatus;
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
    // ---------------------------------
    /// \return a Copy of the class
    virtual MdPackage* Copy() const = 0;
    /// Read package-specific creator options from a file
    virtual int ParseCreatorOption(std::string const&, std::string const&) = 0;
    /// Check creator options
    virtual int CheckCreatorOptions(Creator const&) const = 0;
    /// Read package-specific input options from a file
    virtual int ReadPackageInput(MdOptions&, std::string const&) = 0;
    /// Create package-specific input files
    virtual int CreateInputFiles(Creator const&, int, int, std::string const&, std::string const&) const = 0;
    /// \return Information on an existing run from output files
    virtual RunStatus RunCurrentStatus(std::vector<std::string> const&) const = 0;
    // ---------------------------------
    /// Set debug level
    void SetDebug(int d) { debug_ = d; }
    /// \return debug level
    int Debug() const { return debug_; }
  private:
    int debug_;
};
#endif
