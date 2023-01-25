#include "System.h"
#include "FileRoutines.h"
#include "Messages.h"
#include "Run.h"
#include "StringRoutines.h"
#include "MdPackage.h"

using namespace Messages;

/** CONSTRUCTOR */
System::System() :
  createOptsFilename_("remd.opts"),
  submitOptsFilename_("qsub.opts"),
  runDirPrefix_("run"),
  runDirExtWidth_(3)
{}

/** CONSTRUCTOR - toplevel dir, dirname, description */
System::System(std::string const& top, std::string const& dirname, std::string const& description) :
  topDir_(top),
  dirname_(dirname),
  description_(description),
  createOptsFilename_("remd.opts"),
  submitOptsFilename_("qsub.opts"),
  runDirPrefix_("run"),
  runDirExtWidth_(3)
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
  mdInterface_(rhs.mdInterface_)
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

  return *this;
}

/** Clear all runs. */
void System::clearRuns() {
  Runs_.clear(); 
}

/** Search for run directories in dirname_ */
int System::FindRuns() {
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
    creator_.Info();
  }
  // MD-package specific stuff
  if (mdInterface_.AllocatePackage(MdInterface::AMBER)) {
    ErrorMsg("MD package allocate failed.\n");
    return 1;
  }
  if (!creator_.MdinFileName().empty()) {
    if (mdInterface_.Package()->ReadInputOptions( creator_.MdinFileName() )) {
      ErrorMsg("Reading MD package input options failed.\n");
      return 1;
    }
  }
  // See if submission options exist
  // FIXME re-enable this when the time comes
/*
  if (fileExists( submitOptsFilename_ )) {
    if (submitter_.ReadOptions( submitOptsFilename_ )) {
      ErrorMsg("Reading submission options file name '%s' failed.\n", submitOptsFilename_.c_str());
      return 1;
    }
    if (submitter_.CheckOptions()) {
      ErrorMsg("Checking submission options failed.\n");
      return 1;
    }
  }
*/

  // Search for runs
  StrArray runDirs = ExpandToFilenames(runDirPrefix_ + ".*");
  //if (runDirs.empty()) return 1;

  clearRuns(); 
  Msg("Run directories:\n");
  for (StrArray::const_iterator it = runDirs.begin(); it != runDirs.end(); ++it)
  {
    Msg("  Directory: %s\n", it->c_str());
    // Set up the directory
    Runs_.push_back( Run() );
    if (Runs_.back().SetupExisting( *it, mdInterface_.Package() )) {
      ErrorMsg("Setting up existing run '%s' failed.\n", it->c_str());
      return 1;
    }
    // Change directory back
    if (ChangeToSystemDir()) return 1;
  }

  return 0;
}

/** Set debug level */
void System::SetDebug(int debugIn) {
  creator_.SetDebug( debugIn );
}

/** Print system information. */
void System::PrintInfo() const {
  Msg("%s : %s (%zu runs)\n", dirname_.c_str(), description_.c_str(), Runs_.size());
  // DEBUG
  //Msg("DEBUG\tTop dir: %s\n", topDir_.c_str());
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

/** Create run directories in system directory. */
int System::CreateRunDirectories(std::string const& crd_dir,
                                 int start_run, int nruns, bool overwrite)
{
  using namespace FileRoutines;
  if (nruns < 1) {
    ErrorMsg("Less than 1 run for CreateRunDirectories()\n");
    return 1;
  }
  // Set alternate coords if needed.
  if (!crd_dir.empty())
    creator_.SetSpecifiedCoords( crd_dir );
  creator_.Info();
  // If any runs exist, determine the lowest index
  int lowest_run_idx;
  if (Runs_.empty())
    lowest_run_idx = start_run;
  else {
    lowest_run_idx = Runs_.front().RunIndex();
    Msg("Lowest existing run index is %i\n", lowest_run_idx);
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
      else {
        Runs_[existingRunIdx].RunInfo();
        prevDir = Runs_[existingRunIdx].RunDirName();
        continue;
      }
    }
    // Allocate run
    std::string runDir( runDirPrefix_ + "." + StringRoutines::integerToString(runNum, runWidth) );
    
    if (ChangeToSystemDir()) return 1;

    Msg("  RUNDIR: %s\n", runDir.c_str());
    if (fileExists(runDir) && !overwrite) {
      ErrorMsg("Directory '%s' exists and 'overwrite' not specified.\n", runDir.c_str());
      return 1;
    }
    int create_stat = 0;
    if (existingRunIdx > -1)
      create_stat = Runs_[existingRunIdx].CreateNew(runDir, creator_, mdInterface_.Package(),
                                                    lowest_run_idx, runNum, prevDir);
    else {
      Runs_.push_back( Run() );
      create_stat = Runs_.back().CreateNew(runDir, creator_, mdInterface_.Package(),
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
