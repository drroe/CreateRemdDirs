#include "CpptrajInterface.h"
#include "Messages.h"
#include "StringRoutines.h"
#include <cstdio> // popen

using namespace Messages;

/// Global path for cpptraj execution
static std::string CpptrajInterface_path_ = "";

static bool CpptrajInterface_needs_setup_= true;

/** \return Shell command.
  * Explicitly use bash since it allows stderr redirect to stdout.
  */
std::string CpptrajInterface::shellCmd(std::string const& cmd) const {
  return "/bin/bash -c \"" + cmd + " 2>&1\"";
}

/** CONSTRUCTOR */
CpptrajInterface::CpptrajInterface() {
  if (CpptrajInterface_needs_setup_) {
    CpptrajInterface_needs_setup_ = false;
    std::string cmd = shellCmd("which cpptraj");
    Msg("DEBUG: Command '%s'\n", cmd.c_str());
    FILE* infile = popen(cmd.c_str(), "r");
    if (infile == 0) {
      perror("Error is:");
      ErrorMsg("Command '%s' failed.\n", cmd.c_str());
      return;
    }
    char buffer[1024];
    if (fgets(buffer, 1023, infile) == 0) {
      perror("Error is:");
      ErrorMsg("Getting cpptraj path failed.\n");
      pclose( infile );
      return;
    }
    // If string does not begin with a slash, no file found.
    if (buffer[0] == '/') {
      CpptrajInterface_path_ = StringRoutines::NoTrailingWhitespace(std::string(buffer));
      Msg("DEBUG: Cpptraj path: '%s'\n", CpptrajInterface_path_.c_str());
    } else
      Msg("Warning: No cpptraj found.\n");
    pclose( infile );
  }
}

/** \return True if cpptraj is available. */
bool CpptrajInterface::Available() const {
  return (!CpptrajInterface_path_.empty());
}
