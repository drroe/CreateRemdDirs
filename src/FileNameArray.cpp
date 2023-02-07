#include "FileNameArray.h"
#include "Messages.h"
#include "FileRoutines.h"
#include "StringRoutines.h"

using namespace Messages;
using namespace FileRoutines;

/** CONSTRUCTOR - empty */
FileNameArray::FileNameArray() : prevType_(IS_FILE), fileExtWidth_(0) {}

/** CONSTRUCTOR */
FileNameArray::FileNameArray(std::string const& base,
                             std::string const& prevDir,
                             PrevType prevType,
                             std::string const& crd_ext,
                             int fileExtWidth) :
  base_(tildeExpansion(base)),
  prevDir_(prevDir),
  prevType_(prevType),
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

/** \return Array of input coords for multiple MD. */
FileNameArray::Sarray FileNameArray::multi_file(std::string const& Name,
                                                unsigned int nfiles,
                                                PrevType ptype)
const
{
  static const char* prevtypestr[] = {"IS_DIR", "IS_FILE"};
  Msg("DEBUG: multi_file def='%s' nfiles=%u ptype=%s\n", Name.c_str(), nfiles, prevtypestr[ptype]);
  Sarray crd_files;
  crd_files.reserve( nfiles );
  
  if (ptype == IS_DIR) {
    // Expect <Name>/XXX.<crd_ext_>
    for (unsigned int grp=1; grp <= nfiles; grp++) {
      crd_files.push_back(Name + "/" + NumericalExt(grp, nfiles) + "." + crd_ext_);
      Msg("DEBUG: crd %u '%s'\n", grp, crd_files.back().c_str());
      //crd_files.push_back(tildeExpansion(crdDirName + "/" +
      //                                   NumericalExt(grp, nfiles) + "." + crd_ext_));
    }
  } else {
    // Using same file for everything
    Msg("Warning: Using single input coords for multiple replicas/groups.\n");
    for (unsigned int grp = 1; grp <= nfiles; grp++) {
      crd_files.push_back(Name);
      Msg("DEBUG: crd %u '%s'\n", grp, crd_files.back().c_str());
    }
  }

  return crd_files;
}


/** Generate list of name(s) */
int FileNameArray::Generate(unsigned int nfiles, bool is_initial_run) {
  Msg("DEBUG: Generate nfiles=%u base='%s' is_initial=%i\n", nfiles, base_.c_str(), (int)is_initial_run);
  files_.clear();
  if (base_.empty()) return 0;
  if (nfiles == 0) return 0;

  if (is_initial_run) {//runNum_ == startRunNum_) {
    // Initial run. Use base. Base must exist
    std::string base_path = add_path_prefix(base_);
    if (!fileExists( base_path )) {
      ErrorMsg("'%s' not present.\n", base_path.c_str());
      return 1;
    }
    if (nfiles == 1) {
      files_ = Sarray(1, base_path);
    } else {
      // Determine base type
      PrevType ptype;
      if (IsDirectory(base_path))
        ptype = IS_DIR;
      else
        ptype = IS_FILE;
      files_ = multi_file( base_path, nfiles, ptype );
      // Ensure that files exist for the first run
      for (Sarray::const_iterator it = files_.begin(); it != files_.end(); ++it) {
        if (!fileExists( *it )) {
          ErrorMsg("File '%s' not found. Must specify absolute path"
                   " or path relative to system directory.\n", it->c_str());
          return 1;
        }
      }
    }
  } else {
    // Starting after initial run. Use coordinates from previous run.
    // May not yet exist.
    std::string prev_path = add_path_prefix(prevDir_);
    if (nfiles == 1) {
      files_ = Sarray(1, prev_path);
    } else {
      files_ = multi_file( prev_path, nfiles, prevType_ );
    }
  }
  if (files_.empty()) {
    ErrorMsg("No file names generated.\n");
    return 1;
  }

  return 0;
}
