#ifndef INC_MDPACKAGE_H
#define INC_MDPACKAGE_H
#include <string>
/// Abstract base class for various MD packages.
class MdPackage {
  public:
    /// CONSTRUCTOR
    MdPackage() {}
    /// DESTRUCTOR - virtual since inherited
    virtual ~MdPackage() {}
    /// \return a Copy of the class
    virtual MdPackage* Copy() const = 0;
    /// Read package-specific input options from a file
    virtual int ReadInputOptions(std::string const&) = 0;
  private:
};
#endif
