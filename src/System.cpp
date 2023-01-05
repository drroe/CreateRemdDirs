#include "System.h"
#include "FileRoutines.h"
#include "Messages.h"
#include "Run.h"
// ----- Run types -----
#include "Run_SingleMD.h"

using namespace Messages;

/** CONSTRUCTOR */
System::System() {}

/** Clear all runs. */
void System::clearRuns() {
  for (std::vector<Run*>::iterator it = Runs_.begin(); it != Runs_.end(); ++it)
    delete *it;
}

/** DESTRUCTOR */
System::~System() {
  clearRuns();
}

/** CONSTRUCTOR - toplevel dir, dirname, description */
System::System(std::string const& top, std::string const& dirname, std::string const& description) :
  topDir_(top),
  dirname_(dirname),
  description_(description)
{}

/** COPY CONSTRUCTOR */
System::System(System const& rhs) :
  topDir_(rhs.topDir_),
  dirname_(rhs.dirname_),
  description_(rhs.description_)
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
  clearRuns();
  Runs_.reserve( rhs.Runs_.size() );
  for (std::vector<Run*>::const_iterator it = rhs.Runs_.begin(); it != rhs.Runs_.end(); ++it)
    Runs_.push_back( (*it)->Copy() );

  return *this;
}

/** Search for run directories in dirname_ */
int System::FindRuns() {
  using namespace FileRoutines;
  ChangeDir( topDir_ );
  ChangeDir( dirname_ );
  StrArray runDirs = ExpandToFilenames("run.*");
  if (runDirs.empty()) return 1;

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
    }
    if (run == 0) {
      Msg("Warning: Run allocation failed.\n");
    } else {
      run->SetupRun( *it, output_files );
      Runs_.push_back( run );
    }
    // Change directory back
    ChangeDir( topDir_ );
    ChangeDir( dirname_ );
  }

  return 0;
}

/** Print system information. */
void System::PrintInfo() const {
  Msg("%s : %s (%zu runs)\n", dirname_.c_str(), description_.c_str(), Runs_.size());
}
