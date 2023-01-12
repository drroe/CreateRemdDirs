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
      Msg("Warning: Run allocation failed.\n");
    } else {
      run->SetupRun( *it, output_files );
      Runs_.push_back( run );
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

/** \return array of run directory names from start to stop with current digit width */
System::Sarray System::createRunDirNames(int start_run, int stop_run) const {
  int runWidth = std::max( StringRoutines::DigitWidth(stop_run), runDirExtWidth_ );
  Sarray RunDirs;
  for (int run = start_run; run <= stop_run; ++run)
    RunDirs.push_back( runDirPrefix_ + "." + StringRoutines::integerToString(run, runWidth) );
  return RunDirs;
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
  // Setup run dir creator
  bool needsMdin = true; // TODO needed as an option?
  if (creator_.Setup( crd_dir, needsMdin )) {
    ErrorMsg("Run creator setup failed.\n");
    return 1;
  }
  creator_.Info();
  // Loop over desired run numbers 
  if (Runs_.empty()) {
    // No existing runs.
    int stop_run = start_run + nruns - 1;
    Sarray RunDirs = createRunDirNames(start_run, stop_run);
    Msg("Creating %i runs from %i to %i\n", stop_run - start_run + 1, start_run, stop_run);
    int runNum = start_run;
    for (Sarray::const_iterator runDir = RunDirs.begin();
                                runDir != RunDirs.end(); ++runDir, ++runNum)
    {
      // Allocate run
      Run* thisRun = 0;
      if (creator_.IsSetupForMD()) {
        if (creator_.N_MD_Runs() > 1)
          thisRun = Run_MultiMD::Alloc();
        else
          thisRun = Run_SingleMD::Alloc();
      }
      if (thisRun == 0) {
        ErrorMsg("No allocator yet in CreateRunDirectories.\n");
        return 1;
      }
      if (ChangeToSystemDir()) return 1;

      Msg("  RUNDIR: %s\n", runDir->c_str());
      if (fileExists(*runDir) && !overwrite) {
        ErrorMsg("Directory '%s' exists and 'overwrite' not specified.\n", runDir->c_str());
        return 1;
      }

      if (thisRun->CreateRunDir(creator_, start_run, runNum, *runDir)) {
        ErrorMsg("Creating run '%s' failed.\n", runDir->c_str());
        return 1;
      }
      Runs_.push_back( thisRun );
    }
    //if (creator_.CreateRuns(topDir_ + "/" + dirname_, RunDirs, start_run, overwrite))
    //  return 1;
  } else {
    Msg("NOT YET SET UP FOR HAVING EXISTING RUNS.\n");
  }
  return 0;
}
