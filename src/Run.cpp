#include "Run.h"
#include "Messages.h"
#include "FileRoutines.h"
#include "StringRoutines.h"
#include "MdPackage.h"
#include "Submitter.h"
#include "TextFile.h"
#include "CommonOptions.h"

using namespace Messages;

/** CONSTRUCTOR */
Run::Run() :
  idx_(-1),
  debug_(0)
{}

/** Setup existing run dir (no changes). Assumes we are in that dir. FIXME should it?*/
int Run::SetupExisting(std::string const& runDir, MdPackage* mdpackage)
{
  using namespace FileRoutines;
  if (ChangeDir( runDir )) return 1;

  rundir_ = runDir;
  idx_ = StringRoutines::convertToInteger( Extension(runDir) );
  setupDir_ = GetWorkingDir();
  Msg("DEBUG: Run::SetupExisting: '%s' '%s' '%i'\n", rundir_.c_str(), setupDir_.c_str(), idx_);

  // Get list of files
  StrArray all_files = FileRoutines::ExpandToFilenames("*", false);

  if (all_files.empty()) {
      Msg("Warning: Run directory '%s' is empty.\n", rundir_.c_str());
    runStat_ = RunStatus(RunStatus::EMPTY);
  } else {
    //Msg("DEBUG: Existing files:\n");
    //for (StrArray::const_iterator it = all_files.begin(); it != all_files.end(); ++it)
    //  Msg("\t%s\n", it->c_str());
    runStat_ = mdpackage->RunCurrentStatus( all_files );
    // DEBUG
    //runStat_.Opts().PrintOpts(false, -1, -1);
    // If the status is not COMPLETE and no job id set, see if there is a job id file.
    if (runStat_.CurrentStat() != RunStatus::COMPLETE) {
      if (jobid_.empty()) {
        for (StrArray::const_iterator it = all_files.begin(); it != all_files.end(); ++it)
        {
          if (*it == CommonOptions::Opt_JobIdFilename().Val()) {
            TextFile jfile;
            if (jfile.OpenRead( *it )) return 1;
            jobid_ = jfile.GetString();
            jfile.Close();
          }
        }
      }
      Msg("DEBUG: Job ID is %i\n", jobid_.c_str());

    }
  }

  return 0;
}

/** Refresh existing run. */
int Run::Refresh(MdPackage* mdpackage) {
  using namespace FileRoutines;
  if (ChangeDir( rundir_ )) return 1;
  // Get list of files
  StrArray all_files = FileRoutines::ExpandToFilenames("*", false);
  // Set status
  if (all_files.empty()) {
    //Msg("Warning: Run directory '%s' is empty.\n", rundir_.c_str());
    runStat_ = RunStatus(RunStatus::EMPTY);
  } else {
    runStat_ = mdpackage->RunCurrentStatus( all_files );
  }
  return 0;
}

/** Create new or overwrite existing run dir. */
int Run::CreateNew(std::string const& runDir, Creator const& creator, Submitter const& submitter, MdPackage* mdpackage,
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

  if (mdpackage->CreateInputFiles(creator, submitter, start_run, run_num, rundir_, prevDir)) {
    ErrorMsg("Error creating run in %s\n", runDir.c_str());
    return 1;
  }
  // Set status
  //runStat_ = mdpackage->RunCurrentStatus( FileRoutines::ExpandToFilenames("*", false) );
  runStat_ = RunStatus(RunStatus::PENDING);
  return 0;
}

/** Submit run. */
int Run::SubmitRun(Submitter const& submit, bool first_to_submit, std::string const& prev_jobidIn, std::string const& next_dir, bool testOnly) {
  using namespace FileRoutines;
  if (!fileExists(rundir_)) {
    ErrorMsg("Cannot submit run, directory %s does not exist.\n", rundir_.c_str());
    return 1;
  }
  if (ChangeDir(rundir_)) return 1;

  if ( submit.SubmitJob(jobid_, prev_jobidIn, first_to_submit, idx_, next_dir, testOnly) ) {
    ErrorMsg("Job submission from %s failed.\n", rundir_.c_str());
    return 1;
  }
  Msg("DEBUG: job id = %s  previous job id = %s\n", jobid_.c_str(), prev_jobidIn.c_str());
  // TODO run status
  return 0;
}

/** Print run info to stdout. */
void Run::RunSummary() const {
  Msg("%-8s : ", rundir_.c_str());
  runStat_.PrintStatus();
  Msg("\n");
}
