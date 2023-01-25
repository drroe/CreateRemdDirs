#include "CpptrajInterface.h"
#include "Messages.h"
#include "StringRoutines.h"
#include <cstdio> // popen

using namespace Messages;

/// Global path for cpptraj execution
static std::string CpptrajInterface_path_ = "";

/** \return Shell command.
  * Explicitly use bash since it allows stderr redirect to stdout.
  */
std::string CpptrajInterface::shellCmd(std::string const& cmd) const {
  return "/bin/bash -c \"" + cmd + " 2>&1\"";
}

/** CONSTRUCTOR */
CpptrajInterface::CpptrajInterface() {
  if (CpptrajInterface_path_.empty()) {
    std::string cmd = shellCmd("which cpptraj");
    FILE* pipe = popen(cmd.c_str(), "rb");
    if (pipe == 0) {
      ErrorMsg("Command '%s' failed.\n", cmd.c_str());
      return;
    }
    char buffer[1024];
    if (fgets(buffer, 1023, pipe) == 0) {
      ErrorMsg("Getting cpptraj path failed.\n");
      return;
    }
    CpptrajInterface_path_ = StringRoutines::NoTrailingWhitespace(std::string(buffer));
    pclose( pipe );
    Msg("DEBUG: Cpptraj path: '%s'\n", CpptrajInterface_path_.c_str());
  }
}
