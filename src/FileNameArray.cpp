#include "FileNameArray.h"
#include "Messages.h"
#include "FileRoutines.h"
#include "StringRoutines.h"

using namespace Messages;
using namespace FileRoutines;

/** CONSTRUCTOR */
FileNameArray::FileNameArray(std::string const& base,
                             int startRunNum, int runNum,
                             std::string const& prevDir,
                             std::string const& crd_ext,
                             int fileExtWidth) :
  base_(base),
  startRunNum_(startRunNum),
  runNum_(runNum),
  prevDir_(prevDir),
  crd_ext_(crd_ext),
  fileExtWidth_(fileExtWidth)
{}

/** CONSTRUCTOR - always use base */
FileNameArray::FileNameArray(std::string const& base,
                             std::string const& crd_ext,
                             int fileExtWidth) :
  base_(base),
  startRunNum_(0),
  runNum_(0),
  crd_ext_(crd_ext),
  fileExtWidth_(fileExtWidth)
{}

/** \return File numerical prefix/extension.
  * Determines a numerical prefix/extension based on max number of expected
  * files and the current default width.
  */
std::string FileNameArray::NumericalExt(int num, int max) const {
  int width = std::max(StringRoutines::DigitWidth(max), fileExtWidth_);
  return StringRoutines::integerToString(num, width);
}

/** \return Array containing single input coords. */
FileNameArray::Sarray FileNameArray::single_file(std::string const& specified,
                                                 std::string const& def)
const
{
  Msg("DEBUG: single_file spec='%s'  def='%s'\n", specified.c_str(), def.c_str());
  std::string crdName;
  if (!specified.empty())
    crdName = specified;
  else
    crdName = def;
  // TODO warn if crdName is a directory
  if (crdName.empty()) {
    ErrorMsg("No coordinates file specified.\n");
    return Sarray();
  }
  return Sarray(1, crdName );
  //return Sarray(1, tildeExpansion(crdName) );
}

/** \return Array of input coords for multiple MD. */
FileNameArray::Sarray FileNameArray::multi_file(std::string const& specified,
                                                std::string const& def,
                                                unsigned int nfiles)
const
{
  Msg("DEBUG: multi_file spec='%s'  def='%s'\n", specified.c_str(), def.c_str());
  Sarray crd_files;
  crd_files.reserve( nfiles );
  std::string crdDirName;
  if (!specified.empty())
    crdDirName = specified;
  else
    crdDirName = def;
  if (crdDirName.empty()) {
    ErrorMsg("No coordinates directory specified.\n");
    return Sarray();
  }
  if (IsDirectory( crdDirName )) {
    // Expect <crdDirName>/XXX.<crd_ext_>
    for (unsigned int grp=1; grp <= nfiles; grp++) {
      crd_files.push_back(crdDirName + "/" + NumericalExt(grp, nfiles) + "." + crd_ext_);
      Msg("DEBUG: crd %u '%s'\n", grp, crd_files.back().c_str());
      //crd_files.push_back(tildeExpansion(crdDirName + "/" +
      //                                   NumericalExt(grp, nfiles) + "." + crd_ext_));
    }
  } else {
    // Using same file for everything
    Msg("Warning: Using single input coords for multiple replicas/groups.\n");
    for (unsigned int grp = 1; grp <= nfiles; grp++) {
      crd_files.push_back(crdDirName);
      Msg("DEBUG: crd %u '%s'\n", grp, crd_files.back().c_str());
    }
  }

  return crd_files;
}


/** Generate list of name(s) */
int FileNameArray::Generate(unsigned int nfiles, std::string const& specified_crd) {
  files_.clear();
  if (base_.empty()) return 0;
  if (nfiles == 0) return 0;

  if (runNum_ == 0) {
    // Initial run. Use base.
    if (nfiles == 1) {
      files_ = single_file( add_path_prefix(specified_crd), add_path_prefix(base_) );
    } else {
      files_ = multi_file( add_path_prefix(specified_crd), add_path_prefix(base_), nfiles );
    }
  } else {
    // Starting after run 0. If this is the first run in the series or
    // no previous directory is set and specified_crd_ is set, use that;
    // otherwise set up to use coordinates from previous runs.
    std::string specified;
    if (startRunNum_ == runNum_ || prevDir_.empty())
      specified = specified_crd;
    if (nfiles == 1) {
      files_ = single_file( add_path_prefix(specified), add_path_prefix(prevDir_) );
    } else {
      files_ = multi_file( add_path_prefix(specified), add_path_prefix(prevDir_), nfiles );
    }
  }
  if (files_.empty()) {
    ErrorMsg("No file names generated.\n");
    return 1;
  }

  // Ensure that files exist for the first run
  if (startRunNum_ == runNum_) {
    for (Sarray::const_iterator it = files_.begin(); it != files_.end(); ++it) {
      if (!fileExists( *it )) {
        ErrorMsg("File '%s' not found. Must specify absolute path"
                 " or path relative to system directory.\n", it->c_str());
        return 1;
      }
    }
  }

  return 0;
}
