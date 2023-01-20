#include "MdPackage_Amber.h"
#include "FileRoutines.h"
#include "Messages.h"

using namespace Messages;

/** CONSTRUCTOR */
MdPackage_Amber::MdPackage_Amber() :
  override_ntx_(false),
  override_irest_(false)
{}

/** COPY CONSTRUCTOR */
MdPackage_Amber::MdPackage_Amber(MdPackage_Amber const& rhs) :
  MdPackage(rhs),
  additionalInput_(rhs.additionalInput_),
  override_ntx_(rhs.override_ntx_),
  override_irest_(rhs.override_irest_),
  mdinFile_(rhs.mdinFile_)
{}

/** ASSIGMENT */
MdPackage_Amber& MdPackage_Amber::operator=(MdPackage_Amber const& rhs) {
  if (&rhs == this) return *this;
  MdPackage::operator=(rhs);
  additionalInput_ = rhs.additionalInput_;
  override_ntx_ = rhs.override_ntx_;
  override_irest_ = rhs.override_irest_;
  mdinFile_ = rhs.mdinFile_;

  return *this;
}

/** Read amber-specific input from MDIN file. */
int MdPackage_Amber::ReadInputOptions(std::string const& fname) {
  using namespace FileRoutines;

  if (CheckExists("Amber MDIN file", fname)) return 1;
  std::string mdin_fileName = tildeExpansion(fname);

  override_irest_ = false;
  override_ntx_ = false;
  additionalInput_.clear();
  if (mdinFile_.ParseFile( mdin_fileName )) return 1;
  if (Debug() > 0) mdinFile_.PrintNamelists();
  std::string valname = mdinFile_.GetNamelistVar("&cntrl", "irest");
  if (!valname.empty()) {
    Msg("Warning: Using 'irest = %s' in '%s'\n", valname.c_str(), mdin_fileName.c_str());
    override_irest_ = true;
  }
  valname = mdinFile_.GetNamelistVar("&cntrl", "ntx");
  if (!valname.empty()) {
    Msg("Warning: Using 'ntx = %s' in '%s'\n", valname.c_str(), mdin_fileName.c_str());
    override_ntx_ = true;
  }
  // Add any &cntrl variables to additionalInput_
  for (MdinFile::const_iterator nl = mdinFile_.nl_begin(); nl != mdinFile_.nl_end(); ++nl)
  {
    if (nl->first == "&cntrl") {
      unsigned int col = 0;
      for (MdinFile::token_iterator tkn = nl->second.begin(); tkn != nl->second.end(); ++tkn)
      {
        // Avoid vars which will be set
        if (tkn->first == "imin" ||
            tkn->first == "nstlim" ||
            tkn->first == "dt" ||
            tkn->first == "ig" ||
            tkn->first == "temp0" ||
            tkn->first == "tempi" ||
            tkn->first == "numexchg" ||
            tkn->first == "solvph"
           )
        {
          Msg("Warning: Not using variable '%s' found in '%s'\n", tkn->first.c_str(), mdin_fileName.c_str());
          continue;
        }
        if (col == 0)
          additionalInput_.append("   ");
        
        additionalInput_.append( tkn->first + " = " + tkn->second + ", " );
        col++;
        if (col == 4) {
          additionalInput_.append("\n");
          col = 0;
        }
      }
      if (col != 0)
        additionalInput_.append("\n");
    } else
      Msg("Warning: MDIN file contains additonal namelist '%s'\n", nl->first.c_str());
  }

  if (override_irest_ != override_ntx_) {
    ErrorMsg("Both 'irest' and 'ntx' must be in '%s' if either are.\n", mdin_fileName.c_str());
    return 1;
  }

  return 0;
}
