#include "Run.h"
#include "Messages.h"
#include "FileRoutines.h"
#include "StringRoutines.h"
#include "MdPackage.h"

using namespace Messages;

/** CONSTRUCTOR */
Run::Run() :
  idx_(-1),
  debug_(0)
{}

/** Setup existing run dir (no changes). Assumes we are in that dir. FIXME should it?*/
int Run::SetupExisting(std::string const& runDir)
{
  using namespace FileRoutines;
  rundir_ = runDir;
  idx_ = StringRoutines::convertToInteger( Extension(runDir) );
  setupDir_ = GetWorkingDir();
  Msg("DEBUG: Run::SetupExisting: '%s' '%s' '%i'\n", rundir_.c_str(), setupDir_.c_str(), idx_);
  return 0;
}

/** Create new or overwrite existing run dir. */
int Run::CreateNew(std::string const& runDir, Creator const& creator, MdPackage* mdpackage,
                   int start_run, int run_num, std::string const& prevDir)
{
  using namespace FileRoutines;
  rundir_ = runDir;
  idx_ = StringRoutines::convertToInteger( Extension(runDir) );
  if (!fileExists(rundir_)) {
    // Create
    if (Mkdir(rundir_)) return 1;
  }
  if (ChangeDir(rundir_)) return 1;
  setupDir_ = GetWorkingDir();

  return mdpackage->CreateInputFiles(creator, start_run, run_num, rundir_, prevDir);
}

/** Print run info to stdout. */
void Run::RunInfo() const {
  Msg("Placeholder.\n"); //FIXME
}
