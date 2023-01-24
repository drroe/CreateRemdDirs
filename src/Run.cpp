#include "Run.h"
#include "Messages.h"
#include "FileRoutines.h"
#include "StringRoutines.h"

using namespace Messages;

/** CONSTRUCTOR */
Run::Run() :
  idx_(-1),
  debug_(0)
{}

/** Setup existing run dir. Assumes we are in that dir. */
int Run::SetupExisting(std::string const& runDir)
{
  using namespace FileRoutines;
  rundir_ = runDir;
  setupDir_ = GetWorkingDir();
  idx_ = StringRoutines::convertToInteger( Extension(runDir) );
  Msg("DEBUG: Run::SetupExisting: '%s' '%s' '%i'\n", rundir_.c_str(), setupDir_.c_str(), idx_);
  return 0;
}
