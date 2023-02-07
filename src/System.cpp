#include "System.h"
#include "FileRoutines.h"
#include "Messages.h"
#include "Run.h"
#include "StringRoutines.h"
#include "MdPackage.h"
#include "TextFile.h"

using namespace Messages;

/** CONSTRUCTOR */
System::System() :
  createOptsFilename_("remd.opts"),
  submitOptsFilename_("qsub.opts"),
  runDirPrefix_("run"),
  runDirExtWidth_(3),
  c_needs_save_(false),
  s_needs_save_(false)
{}

/** CONSTRUCTOR - toplevel dir, dirname, description */
System::System(std::string const& top, std::string const& dirname, std::string const& description) :
  topDir_(top),
  dirname_(dirname),
  description_(description),
  createOptsFilename_("remd.opts"),
  submitOptsFilename_("qsub.opts"),
  runDirPrefix_("run"),
  runDirExtWidth_(3),
  c_needs_save_(false),
  s_needs_save_(false)
{}

/** COPY CONSTRUCTOR */
System::System(System const& rhs) :
  Runs_(rhs.Runs_),
  topDir_(rhs.topDir_),
  dirname_(rhs.dirname_),
  description_(rhs.description_),
  createOptsFilename_(rhs.createOptsFilename_),
  submitOptsFilename_(rhs.submitOptsFilename_),
  runDirPrefix_(rhs.runDirPrefix_),
  runDirExtWidth_(rhs.runDirExtWidth_),
  creator_(rhs.creator_),
  submitter_(rhs.submitter_),
  mdInterface_(rhs.mdInterface_),
  c_needs_save_(rhs.c_needs_save_),
  s_needs_save_(rhs.s_needs_save_)
{}

/** Assignment */
System& System::operator=(System const& rhs) {
  if (this == &rhs) return *this;
  Runs_ = rhs.Runs_;
  topDir_ = rhs.topDir_;
  dirname_ = rhs.dirname_;
  description_ = rhs.description_;
  createOptsFilename_ = rhs.createOptsFilename_;
  submitOptsFilename_ = rhs.submitOptsFilename_;
  runDirPrefix_ = rhs.runDirPrefix_;
  runDirExtWidth_ = rhs.runDirExtWidth_;
  creator_ = rhs.creator_;
  submitter_ = rhs.submitter_;
  mdInterface_ = rhs.mdInterface_;
  c_needs_save_ = rhs.c_needs_save_;
  s_needs_save_ = rhs.s_needs_save_;

  return *this;
}

/** Clear all runs. */
void System::clearRuns() {
  Runs_.clear(); 
}

/** Write options to files. */
int System::WriteSystemOptions() {
  using namespace FileRoutines;
  if (ChangeToSystemDir()) {
    ErrorMsg("Could not change to system directory %s/%s\n", topDir_.c_str(), dirname_.c_str());
    return 1;
  }
  // ----- Creator and MdPackage -------
  if (c_needs_save_) {
    bool write_file = true;
    if (fileExists( createOptsFilename_ )) {
      Msg("Warning: '%s' exists.\n", createOptsFilename_.c_str());
      write_file = YesNoPrompt("Overwrite?");
    }
    if (write_file) {
      Msg("Writing create options to '%s'\n", createOptsFilename_.c_str());
      TextFile outfile;
      if (outfile.OpenWrite( createOptsFilename_ )) {
        ErrorMsg("Opening '%s' for write failed.\n", createOptsFilename_.c_str());
        return 1;
      }
      if (creator_.WriteOptions( outfile )) {
        ErrorMsg("Writing creation options to file '%s' in dir '%s' failed.\n", createOptsFilename_.c_str(), dirname_.c_str());
        return 1;
      }
      if (mdInterface_.HasPackage()) {
        if (mdInterface_.Package()->WriteCreatorOptions( outfile )) {
          ErrorMsg("Writing package-specific creation options to file '%s' in dir '%s' failed.\n", createOptsFilename_.c_str(), dirname_.c_str());
          return 1;
        }
      }
      outfile.Close();
      c_needs_save_ = false;
    }
  }
  // ----- Submitter and Queue ---------
  if (s_needs_save_) {
    bool write_file = true;
    if (fileExists( submitOptsFilename_ )) {
      Msg("Warning: '%s' exists.\n", submitOptsFilename_.c_str());
      write_file = YesNoPrompt("Overwrite?");
    }
    if (write_file) {
      Msg("Writing submit options to '%s'\n", submitOptsFilename_.c_str());
      TextFile outfile;
      if (outfile.OpenWrite( submitOptsFilename_ )) {
        ErrorMsg("Opening '%s' for write failed.\n", submitOptsFilename_.c_str());
        return 1;
      }
      if (submitter_.WriteOptions( outfile )) {
        ErrorMsg("Writing submit options to file '%s' in dir '%s' failed.\n", submitOptsFilename_.c_str(), dirname_.c_str());
        return 1;
      }
      outfile.Close();
      s_needs_save_ = false;
    }
  }

  return 0;
}

/** Read MD package-specific input if needed. This is in a separate routine
  * because it needs to be done after the initial creation options read
  * (so that the MdPackage is actually allocated).
  */
int System::read_mdpackage_mdin() {
  if (!creator_.MdinNeedsRead()) {
    Msg("DEBUG: MDIN does not need reading. Skipping.\n");
    return 0;
  }
  if (!creator_.MdinFileName().empty()) {
    MdOptions packageOpts;
    if (mdInterface_.Package()->ReadPackageInput( packageOpts, creator_.MdinFileName() )) {
      ErrorMsg("Reading MD package input options from '%s' failed.\n", creator_.MdinFileName().c_str());
      return 1;
    }
    // See if we want to use any of the package options
    if (creator_.SetMdOptions( packageOpts )) {
      ErrorMsg("Setting MD options from package options failed.\n");
      return 1;
    }
  }

  creator_.Set_MdinAsRead();
  return 0;
}

/** Search for run directories in dirname_ */
int System::FindRuns(QueueArray& queues) {
  using namespace FileRoutines;
  if (ChangeToSystemDir()) {
    ErrorMsg("Could not change to system directory %s/%s\n", topDir_.c_str(), dirname_.c_str());
    return 1;
  }

  // See if creation options exist
  if (fileExists( createOptsFilename_ )) {
    if (creator_.ReadOptions( createOptsFilename_ )) {
      ErrorMsg("Reading creation options file name '%s' in dir '%s' failed.\n", createOptsFilename_.c_str(), dirname_.c_str());
      return 1;
    }
  }
  // Allocate specific MD package
  if (mdInterface_.AllocatePackage(MdInterface::AMBER, creator_.Debug())) {
    ErrorMsg("MD package allocate failed.\n");
    return 1;
  }
  // Process MD package-specific options
  for (OptArray::const_iterator opair = creator_.PackageOpts().begin();
                                opair != creator_.PackageOpts().end(); ++opair)
  {
    std::string const& OPT = opair->first;
    std::string const& VAR = opair->second;
    int ret = mdInterface_.Package()->ParseCreatorOption(OPT, VAR);
    if (ret == -1) {
      ErrorMsg("Could not parse package-specific option '%s' = '%s'\n", OPT.c_str(), VAR.c_str());
      return 1;
    } else if (ret == 0) {
      Msg("Warning: Ignoring unrecognized option '%s' = '%s'\n", OPT.c_str(), VAR.c_str());
    }
  }
  // Process MD package-specific MD input if needed
  if (read_mdpackage_mdin()) return 1;
  // Check the options
  if (creator_.CheckCreator()) {
    Msg("Warning: Invalid Creator options detected.\n");
  }
  if (mdInterface_.Package()->CheckCreatorOptions(creator_)) {
    Msg("Warning: Invalid package-specific Creator options.\n");
  }
  creator_.Info();

  // See if submission options exist
  if (fileExists( submitOptsFilename_ )) {
    if (submitter_.ReadOptions( submitOptsFilename_ )) {
      ErrorMsg("Reading submission options file name '%s' failed.\n", submitOptsFilename_.c_str());
      return 1;
    }
    /*if (submitter_.CheckOptions()) {
      ErrorMsg("Checking submission options failed.\n");
      return 1;
    }*/
  }
  if (submitter_.CheckSubmitter()) {
    Msg("Warning: Invalid Submitter options detected.\n");
  }
  if (mdInterface_.Package()->CheckSubmitterOptions(creator_, submitter_)) {
    Msg("Warning: Invalid package-specific Submitter options.\n");
  }
  submitter_.Info();

  // Search for runs
  StrArray runDirs = ExpandToFilenames(runDirPrefix_ + ".*");
  //if (runDirs.empty()) return 1;

  clearRuns(); 
  Msg("Run directories:\n");
  int lastIdx = -1;
  for (StrArray::const_iterator it = runDirs.begin(); it != runDirs.end(); ++it)
  {
    Msg("  Directory: %s\n", it->c_str());
    // Set up the directory
    Runs_.AddRun( Run() );
    if (Runs_.Set_back().SetupExisting( *it, mdInterface_.Package() )) {
      ErrorMsg("Setting up existing run '%s' failed.\n", it->c_str());
      return 1;
    }
    // Check that runs are consecutive
    if (lastIdx != -1) {
      if (Runs_.back().RunIndex() != lastIdx + 1)
        Msg("Warning: Runs %i and %i are not consecutive.\n", Runs_.back().RunIndex(), lastIdx);
    }
    lastIdx = Runs_.back().RunIndex();

    // Change directory back
    if (ChangeToSystemDir()) return 1;
  }

  return 0;
}

/** Parse a single option.
  * \return 1 if processed, 0 if ignored, -1 if error.
  */
int System::ParseOption(std::string const& OPT, std::string const& VAR) {
  OptArray::OptPair opair(OPT, VAR);
  // Creator
  int ret = creator_.ParseFileOption( opair );
  if (ret == 1) {
    // Ensure creator is refreshed after parsing the option
    creator_.CheckCreator();
    mdInterface_.Package()->CheckCreatorOptions(creator_);
    c_needs_save_ = true;
  }
  // Md package
  if (ret == 0) {
    ret = mdInterface_.Package()->ParseCreatorOption( OPT, VAR );
    if (ret == 1) {
      mdInterface_.Package()->CheckCreatorOptions(creator_);
      c_needs_save_ = true;
    }
  }
  // Submitter
  if (ret == 0) {
    ret = submitter_.ParseFileOption( opair );
    if (ret == 1) {
      // Ensure submitter is checked after parsing the option
      submitter_.CheckSubmitter();
      mdInterface_.Package()->CheckSubmitterOptions(creator_, submitter_);
      s_needs_save_ = true;
    }
  }
  // Handle return status
  if (ret == 1) {
    // Process MD package-specific MD input if needed
    if (read_mdpackage_mdin()) return -1;
  }
  return ret;
}

/** Refresh current runs. */
int System::RefreshCurrentRuns(bool verbose) {
  using namespace FileRoutines;
  if (ChangeToSystemDir()) {
    ErrorMsg("Could not change to system directory %s/%s\n", topDir_.c_str(), dirname_.c_str());
    return 1;
  }
  for (RunArray::iterator run = Runs_.begin(); run != Runs_.end(); ++run)
  {
    if (verbose)
      Msg("  Refreshing directory '%s'\n", run->RunDirName().c_str());
    if (run->Refresh( mdInterface_.Package() )) {
      ErrorMsg("Refreshing existing run '%s'\n", run->RunDirName().c_str());
      return 1;
    }
    if (verbose)
      run->RunSummary();
    // Change directory back
    if (ChangeToSystemDir()) return 1;
  }
  return 0;
}

/** Set debug level */
void System::SetDebug(int debugIn) {
  creator_.SetDebug( debugIn );
  submitter_.SetDebug( debugIn );
}

/** Print system summary. */
void System::PrintSummary() const {
  Msg("%s : %s (%zu runs)\n", dirname_.c_str(), description_.c_str(), Runs_.size());
  // DEBUG
  //Msg("DEBUG\tTop dir: %s\n", topDir_.c_str());
}

/** Print system info. */
void System::PrintInfo() const {
  creator_.Info();
  if (mdInterface_.HasPackage())
    mdInterface_.Package()->PackageInfo();
  submitter_.Info();
}

/** Change to system directory. */
int System::ChangeToSystemDir() const {
  using namespace FileRoutines;
  if (ChangeDir( topDir_ )) return 1;
  if (ChangeDir( dirname_ )) return 1;
  return 0;
}

/** \return Index of run with specified runNum or -1 if not present. */
int System::findRunIdx(int runNum) const {
  int existingRunIdx = -1;
  for (unsigned int ridx = 0; ridx < Runs_.size(); ridx++) {
    if ( Runs_[ridx].RunIndex() == runNum) {
      Msg(" Run %i exists.\n", runNum);
      existingRunIdx = (int)ridx;
      break;
    }
  }
  return existingRunIdx;
}

/** Submit runs in system directory. */
int System::SubmitRunDirectories(int start_run, int nruns, bool overwrite,
                                 std::string const& prev_jobidIn)
{
  if (nruns < 1) {
    ErrorMsg("Less than 1 run for SubmitRunDirectories()\n");
    return 1;
  }
  if (start_run < 0) {
    ErrorMsg("Start run index is negative.\n");
    return 1;
  }
  // Ensure the submitter is valid
  submitter_.Info();
  if (submitter_.CheckSubmitter() || 
      mdInterface_.Package()->CheckSubmitterOptions(creator_, submitter_))
  {
    ErrorMsg("Invalid options detected. Cannot submit.\n");
    return 1;
  }
  // Loop over desired run numbers
  std::string prev_jobid = prev_jobidIn;
  int stop_run = start_run + nruns - 1;
  for (int runNum = start_run; runNum <= stop_run; ++runNum)
  {
    // See if this run already exists
    int existingRunIdx = findRunIdx(runNum);
    if (existingRunIdx < 0) {
      ErrorMsg("Cannot submit run %i, does not exist.\n", runNum);
      return 1;
    }
    // See if next run exists
    std::string next_dir;
    int nextRunIdx = findRunIdx(runNum+1);
    if (nextRunIdx > -1)
      next_dir = Runs_[nextRunIdx].RunDirName();
    // If not overwriting, only submit if pending
    Run& currentRun = Runs_.Set_Run(existingRunIdx);
    if (!overwrite) {
      if (currentRun.Stat().CurrentStat() != RunStatus::PENDING) {
        ErrorMsg("Not overwriting and run %s is not pending.\n", currentRun.RunDirName().c_str());
        return 1;
      }
    }

    if (ChangeToSystemDir()) return 1;
    Msg("Submit ");
    currentRun.RunSummary();
    if (currentRun.SubmitRun( submitter_, prev_jobid, next_dir )) {
      ErrorMsg("Run submission failed.\n");
      return 1;
    }
    prev_jobid = currentRun.JobId();
  }

  return 0;
}

/** Create run directories in system directory. */
int System::CreateRunDirectories(int start_run, int nruns, bool overwrite)
{
  //using namespace FileRoutines;
  if (nruns < 1) {
    ErrorMsg("Less than 1 run for CreateRunDirectories()\n");
    return 1;
  }
  if (start_run < 0) {
    ErrorMsg("Start run index is negative.\n");
    return 1;
  }
  // Ensure the creator is valid
  creator_.Info();
  if (creator_.CheckCreator() || mdInterface_.Package()->CheckCreatorOptions(creator_)) { 
    ErrorMsg("Invalid options detected. Cannot create.\n");
    return 1;
  }
  // If any runs exist, determine the lowest index and highest index
  int lowest_run_idx;
  if (Runs_.empty())
    lowest_run_idx = start_run;
  else {
    lowest_run_idx = Runs_.front().RunIndex();
    Msg("Lowest existing run index is %i\n", lowest_run_idx);
    int highest_run_idx = Runs_.back().RunIndex();
    if (start_run > highest_run_idx + 1) {
      ErrorMsg("Error: Runs exist and specified start index %i is not the next\n"
               "Error:   index after run %i.\n", start_run, highest_run_idx);
      return 1;
    }
    if (start_run < lowest_run_idx) {
      ErrorMsg("Error: Runs exist and specified start index %i is lower than\n"
               "Error:   lowest existing run index %i\n", start_run, lowest_run_idx);
      return 1;
    }
  }
  // Loop over desired run numbers 
  int stop_run = start_run + nruns - 1;
  int runWidth = std::max( StringRoutines::DigitWidth(stop_run), runDirExtWidth_ );
  Msg("Creating %i runs from %i to %i\n", stop_run - start_run + 1, start_run, stop_run);
  std::string prevDir;
  // See if there is a run before start_run
  if (start_run > 0) {
    int prevIdx = findRunIdx(start_run - 1);
    if (prevIdx > -1)
      prevDir = Runs_[prevIdx].RunDirName();
  }
  // Loop over runs
  for (int runNum = start_run; runNum <= stop_run; ++runNum)
  {
    // See if this run already exists
    int existingRunIdx = findRunIdx(runNum);
    if (existingRunIdx > -1) {
      if (overwrite)
        Msg("Will overwrite run.\n");
      else if (Runs_[existingRunIdx].Stat().CurrentStat() == RunStatus::EMPTY)
        Msg("Run directory is empty. Creating run.\n");
      else {
        Runs_[existingRunIdx].RunSummary();
        prevDir = Runs_[existingRunIdx].RunDirName();
        continue;
      }
    }
    // Allocate run
    std::string runDir( runDirPrefix_ + "." + StringRoutines::integerToString(runNum, runWidth) );
    
    if (ChangeToSystemDir()) return 1;

    Msg("  RUNDIR: %s\n", runDir.c_str());
    // NOTE: directory at this point should exist if it is in Runs_ so do not check
    //if (fileExists(runDir) && !overwrite) {
    //  ErrorMsg("Directory '%s' exists and 'overwrite' not specified.\n", runDir.c_str());
    //  return 1;
    //}
    int create_stat = 0;
    if (existingRunIdx > -1)
      create_stat = Runs_.Set_Run(existingRunIdx).CreateNew(runDir, creator_, submitter_, mdInterface_.Package(),
                                                            lowest_run_idx, runNum, prevDir);
    else {
      Runs_.AddRun( Run() );
      create_stat = Runs_.Set_back().CreateNew(runDir, creator_, submitter_, mdInterface_.Package(),
                                               lowest_run_idx, runNum, prevDir);
    }
    if (create_stat != 0) {
      ErrorMsg("Creating run '%s' failed.\n", runDir.c_str());
      return 1;
    }

    prevDir = runDir;
  }
 
  return 0;
}
