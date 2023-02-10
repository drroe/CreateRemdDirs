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
  debug_(0),
  t_last_mod_(-1)
{}

/** Get job id from file. */
int Run::getJobIdFromFile(std::vector<std::string> const& all_files, Queue const& localQueue) {
  if (runStat_.CurrentStat() != RunStatus::COMPLETE) {
    for (std::vector<std::string>::const_iterator it = all_files.begin(); it != all_files.end(); ++it)
    {
      if (*it == CommonOptions::Opt_JobIdFilename().Val()) {
        TextFile jfile;
        if (jfile.OpenRead( *it )) return 1;
        jobid_ = jfile.GetString();
        jfile.Close();
      }
    }
    // If no job id, just exit.
    if (jobid_.empty()) return 0;

    Queue::JobStatType jstat = localQueue.JobStatus( jobid_ );
    //Msg("DEBUG: Job ID is %s  jstat=%i\n", jobid_.c_str(), (int)jstat);
    if (jstat == Queue::QUEUED)
      runStat_.Set_Status(RunStatus::IN_QUEUE);
    else if (jstat == Queue::RUNNING)
      runStat_.Set_Status(RunStatus::IN_PROGRESS);
  }
  return 0;
}

/** Update time last modified. 
  * \return 1 if time was updated, 0 if not.
  */
int Run::updateTimeLastModified(std::vector<std::string> const& all_files) {
  int ret = 0;
  for (std::vector<std::string>::const_iterator it = all_files.begin(); it != all_files.end(); ++it) 
  {
    long int t_lm = FileRoutines::TimeLastModified( *it );
    //Msg("DEBUG\t%s (%li)\n", it->c_str(), t_lm);
    if (t_lm > t_last_mod_) {
      t_last_mod_ = t_lm;
      ret = 1;
    }
  }
  return ret;
}

/** Do all actions necessary to refresh the run. */
int Run::internalRefresh(MdPackage* mdpackage, Queue const& localQueue, std::vector<std::string> const& all_files) {
  // Update time last modified
  int needs_update = updateTimeLastModified( all_files );
  // Only update run status if files have been modified.
  if (needs_update == 1) {
    Msg("DEBUG: Updating status of '%s'\n", rundir_.c_str());
    runStat_ = mdpackage->RunCurrentStatus( all_files );
  }
  // DEBUG
  //runStat_.Opts().PrintOpts(false, -1, -1);
  // If the status is not COMPLETE and no job id set, see if there is a job id file.
  if (getJobIdFromFile(all_files, localQueue)) return 1;
  return 0;
}

/* Setup existing run dir (no changes). Assumes dir exists. */
int Run::SetupExisting(std::string const& runDir, MdPackage* mdpackage, Queue const& localQueue)
{
  using namespace FileRoutines;
  if (ChangeDir( runDir )) return 1;

  rundir_ = runDir;
  idx_ = StringRoutines::convertToInteger( Extension(runDir) );
  setupDir_ = GetWorkingDir();
  if (debug_ > 0)
    Msg("DEBUG: Run::SetupExisting: '%s' '%s' '%i'\n", rundir_.c_str(), setupDir_.c_str(), idx_);

  // Get list of files
  StrArray all_files = FileRoutines::ExpandToFilenames("*", false);

  if (all_files.empty()) {
      Msg("Warning: Run directory '%s' is empty.\n", rundir_.c_str());
    runStat_ = RunStatus(RunStatus::EMPTY);
  } else {
    if (internalRefresh(mdpackage, localQueue, all_files)) return 1;
/*
    //Msg("DEBUG: Existing files:\n");
    // Update time last modified
    updateTimeLastModified( all_files );

    runStat_ = mdpackage->RunCurrentStatus( all_files );
    // DEBUG
    //runStat_.Opts().PrintOpts(false, -1, -1);
    // If the status is not COMPLETE and no job id set, see if there is a job id file.
    if (getJobIdFromFile(all_files, localQueue)) return 1;*/
  }

  return 0;
}

/** Refresh existing run. */
int Run::Refresh(MdPackage* mdpackage, Queue const& localQueue) {
  using namespace FileRoutines;
  if (ChangeDir( rundir_ )) return 1;
  // Get list of files
  StrArray all_files = FileRoutines::ExpandToFilenames("*", false);
  // Set status
  if (all_files.empty()) {
    //Msg("Warning: Run directory '%s' is empty.\n", rundir_.c_str());
    runStat_ = RunStatus(RunStatus::EMPTY);
  } else {
    if (internalRefresh(mdpackage, localQueue, all_files)) return 1;
/*
    // Update time last modified
    updateTimeLastModified( all_files );

    runStat_ = mdpackage->RunCurrentStatus( all_files );
    // If the status is not COMPLETE and no job id set, see if there is a job id file.
    if (getJobIdFromFile(all_files, localQueue)) return 1;*/
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
  runStat_.PrintStatus(jobid_);
  Msg("\n");
}
