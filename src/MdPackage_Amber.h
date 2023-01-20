#ifndef INC_MDPACKAGE_AMBER_H
#define INC_MDPACKAGE_AMBER_H
#include "MdPackage.h"
/// Amber-specific options
class MdPackage_Amber : public MdPackage {
  public:
    /// CONSTRUCTOR
    MdPackage_Amber();
    /// COPY CONSTRUCTOR
    MdPackage_Amber(MdPackage_Amber const&);
    /// ASSIGNMENT
    MdPackage_Amber& operator=(MdPackage_Amber const&);

    /// \return Copy of this class
    MdPackage* Copy() const { return (MdPackage*)new MdPackage_Amber(*this); }
    /// Read amber-specific input options from file
    int ReadInputOptions(std::string const&);
  private:
};
#endif
