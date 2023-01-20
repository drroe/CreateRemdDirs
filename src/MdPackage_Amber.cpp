#include "MdPackage_Amber.h"

/** CONSTRUCTOR */
MdPackage_Amber::MdPackage_Amber() {}

/** COPY CONSTRUCTOR */
MdPackage_Amber::MdPackage_Amber(MdPackage_Amber const& rhs) :
  MdPackage(rhs)
{}

/** ASSIGMENT */
MdPackage_Amber& MdPackage_Amber::operator=(MdPackage_Amber const& rhs) {
  if (&rhs == this) return *this;
  MdPackage::operator=(rhs);
  return *this;
}

/** Read amber-specific input from MDIN file. */
int MdPackage_Amber::ReadInputOptions(std::string const& fname) {
  return 1;
}
