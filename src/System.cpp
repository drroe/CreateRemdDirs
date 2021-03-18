#include "System.h"
#include "FileRoutines.h"
#include "Messages.h"
#include "Run.h"
// ----- Run types -----
#include "Run_Single.h"

using namespace Messages;

/** CONSTRUCTOR */
System::System() {}

/** DESTRUCTOR */
System::~System() {
  for (std::vector<Run*>::iterator it = Runs_.begin(); it != Runs_.end(); ++it)
    delete *it;
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
{}

/** Assignment */
System& System::operator=(System const& rhs) {
  if (this == &rhs) return *this;
  topDir_ = rhs.topDir_;
  dirname_ = rhs.dirname_;
  description_ = rhs.description_;
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
    ChangeDir( topDir_ );
    ChangeDir( dirname_ );

    // Allocate run
    Run* run = 0;
    switch (runType) {
      case Run::SINGLE_MD : run = Run_Single::Alloc(); break;
    }
    if (run == 0) {
      ErrorMsg("Run allocation failed.\n");
      return 1;
    }

    run->SetRunDir( *it );

    Runs_.push_back( run );
  }

  return 0;
}
