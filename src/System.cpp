#include "System.h"
#include "FileRoutines.h"
#include "Messages.h"

using namespace Messages;

/** CONSTRUCTOR */
System::System() {}

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
    Msg("\t%s\n", it->c_str());

  return 0;
}
