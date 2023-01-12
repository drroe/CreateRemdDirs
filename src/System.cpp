#include "System.h"
#include "FileRoutines.h"
#include "Messages.h"
#include "Run.h"
#include "StringRoutines.h"
// ----- Run types -----
#include "Run_SingleMD.h"
#include "Run_MultiMD.h"

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
  topDir_(rhs.topDir_),
  dirname_(rhs.dirname_),
  description_(rhs.description_),
  createOptsFilename_(rhs.createOptsFilename_),
  submitOptsFilename_(rhs.submitOptsFilename_),
  runDirPrefix_(rhs.runDirPrefix_),
  runDirExtWidth_(rhs.runDirExtWidth_)
{
  Runs_.reserve( rhs.Runs_.size() );
  for (std::vector<Run*>::const_iterator it = rhs.Runs_.begin(); it != rhs.Runs_.end(); ++it)
    Runs_.push_back( (*it)->Copy() );
}

/** Assignment */
System& System::operator=(System const& rhs) {
  if (this == &rhs) return *this;
  topDir_ = rhs.topDir_;
  dirname_ = rhs.dirname_;
  description_ = rhs.description_;
  createOptsFilename_ = rhs.createOptsFilename_;
  submitOptsFilename_ = rhs.submitOptsFilename_;
  runDirPrefix_ = rhs.runDirPrefix_;
  runDirExtWidth_ = rhs.runDirExtWidth_;
  clearRuns();
  Runs_.reserve( rhs.Runs_.size() );
  for (std::vector<Run*>::const_iterator it = rhs.Runs_.begin(); it != rhs.Runs_.end(); ++it)
    Runs_.push_back( (*it)->Copy() );

  return *this;
}

/** Clear all runs. */
void System::clearRuns() {
  for (std::vector<Run*>::iterator it = Runs_.begin(); it != Runs_.end(); ++it)
    delete *it;
}

/** DESTRUCTOR */
System::~System() {
  clearRuns();
}

/** Allocate Run based on creator_. */
Run* System::allocateFromCreator(std::string const& runDir) const {
  Run* thisRun = 0;
  if (creator_.IsSetupForMD()) {
    if (creator_.N_MD_Runs() > 1)
      thisRun = Run_MultiMD::Alloc();
    else
      thisRun = Run_SingleMD::Alloc();
  }
  if (thisRun != 0) {
    if (thisRun->SetupRun( runDir )) {
      ErrorMsg("Could not set up run '%s' from run directory.\n", runDir.c_str());
      delete thisRun;
      return 0;
    }
  }

  return thisRun;
}

/** Search for run directories in dirname_ */
int System::FindRuns() {
  using namespace FileRoutines;
  ChangeDir( topDir_ );
  ChangeDir( dirname_ );

  // See if creation options exist
  if (fileExists( createOptsFilename_ )) {
    if (creator_.ReadOptions( createOptsFilename_ )) {
      ErrorMsg("Reading creation options file name '%s' failed.\n", createOptsFilename_.c_str());
      return 1;
    }
    creator_.Info();
  }
  // See if submission options exist
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

  // Search for runs
  StrArray runDirs = ExpandToFilenames(runDirPrefix_ + ".*");
  //if (runDirs.empty()) return 1;

  Msg("Run directories:\n");
  for (StrArray::const_iterator it = runDirs.begin(); it != runDirs.end(); ++it)
  {
    ChangeDir( *it );
    Msg("\t%s\n", it->c_str());
    StrArray output_files;
    Run::Type runType = Run::DetectType(output_files);
    Msg("\tType: %s\n", Run::typeStr(runType));

    // Allocate run
    Run* run = 0;
    switch (runType) {
      case Run::SINGLE_MD : run = Run_SingleMD::Alloc(); break;
      case Run::MULTI_MD  : run = Run_MultiMD::Alloc(); break;
      case Run::REMD      :
      case Run::UNKNOWN   : break;
    }
    if (run == 0) {
      Msg("Warning: Run detection failed. Allocating based on '%s'\n", createOptsFilename_.c_str());
      run = allocateFromCreator( *it );
    } 
    if (run == 0) {
      ErrorMsg("Run allocation failed.\n");
      return 1;
    } else
      Runs_.push_back( run );
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
    lowest_run_idx = Runs_.front()->RunIndex();
    Msg("Lowest existing run index is %i\n", lowest_run_idx);
  }
  // Loop over desired run numbers 
  int stop_run = start_run + nruns - 1;
  int runWidth = std::max( StringRoutines::DigitWidth(stop_run), runDirExtWidth_ );
  Msg("Creating %i runs from %i to %i\n", stop_run - start_run + 1, start_run, stop_run);

    for (int runNum = start_run; runNum <= stop_run; ++runNum)
//    for (Sarray::const_iterator runDir = RunDirs.begin();
//                                runDir != RunDirs.end(); ++runDir, ++runNum)
    {
      // See if this run already exists
      Run const* existingRun = 0;
      for (RunArray::const_iterator it = Runs_.begin(); it != Runs_.end(); ++it) {
        if ((*it)->RunIndex() == runNum) {
          Msg(" Run %i exists.\n", runNum);
          existingRun = *it;
          break;
        }
      }
      if (existingRun != 0) {
        existingRun->RunInfo();
        continue; // TODO if overwrite is set allow existing run to be replaced
      }
      // Allocate run
      std::string runDir( runDirPrefix_ + "." + StringRoutines::integerToString(runNum, runWidth) );
      Run* thisRun = allocateFromCreator( runDir );
      if (thisRun == 0) {
        ErrorMsg("No allocator yet in CreateRunDirectories.\n");
        return 1;
      }
      if (ChangeToSystemDir()) return 1;

      Msg("  RUNDIR: %s\n", runDir.c_str());
      if (fileExists(runDir) && !overwrite) {
        ErrorMsg("Directory '%s' exists and 'overwrite' not specified.\n", runDir.c_str());
        return 1;
      }

      if (thisRun->CreateRunDir(creator_, lowest_run_idx, runNum, runDir)) {
        ErrorMsg("Creating run '%s' failed.\n", runDir.c_str());
        return 1;
      }
      Runs_.push_back( thisRun );
    }
    //if (creator_.CreateRuns(topDir_ + "/" + dirname_, RunDirs, start_run, overwrite))
    //  return 1;
//  } else {
//    Msg("NOT YET SET UP FOR HAVING EXISTING RUNS.\n");
//  }
  return 0;
}
