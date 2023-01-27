#include "CpptrajInterface.h"
#include "Messages.h"
#include "StringRoutines.h"
#include "TextFile.h"

using namespace Messages;

/// Global path for cpptraj execution
static std::string CpptrajInterface_path_ = "";

static bool CpptrajInterface_needs_setup_= true;

/** \return Shell command.
  * Explicitly use bash since it allows stderr redirect to stdout.
  */
std::string CpptrajInterface::shellCmd(std::string const& cmdIn) const {
  std::string cmd( "/bin/bash -c \"" + cmdIn + " 2>&1\"" );
  //Msg("DEBUG: Command '%s'\n", cmd.c_str());
  return cmd;
}

/** CONSTRUCTOR */
CpptrajInterface::CpptrajInterface() {
  if (CpptrajInterface_needs_setup_) {
    CpptrajInterface_needs_setup_ = false;
    std::string cmd = shellCmd("which cpptraj");
    TextFile infile;
    if (infile.OpenPipe( cmd )) {
      ErrorMsg("Command '%s' failed.\n", cmd.c_str());
      return;
    }
    const char* ptr = infile.Gets();
    if (ptr == 0) {
      ErrorMsg("Getting cpptraj path failed.\n");
      return;
    }
    // If string does not begin with a slash, no file found.
    if (ptr[0] == '/') {
      CpptrajInterface_path_ = StringRoutines::NoTrailingWhitespace(std::string(ptr));
      Msg("DEBUG: Cpptraj path: '%s'\n", CpptrajInterface_path_.c_str());
    } else
      Msg("Warning: No cpptraj found.\n");
  }
}

/** \return True if cpptraj is available. */
bool CpptrajInterface::Available() const {
  return (!CpptrajInterface_path_.empty());
}

/** \return Number of frames in top/traj combo */
int CpptrajInterface::GetTrajFrames(std::string const& topname, std::string const& trajname) const {
  std::string cmd = shellCmd( CpptrajInterface_path_ + " -p " + topname + " -y " + trajname + " -tl" );
  TextFile infile;
  if (infile.OpenPipe(cmd)) {
    ErrorMsg("Command '%s' failed.\n", cmd.c_str());
    return -1;
  }
  //                     0123456789
  // Expected output is 'Frames: #'
  const char* ptr = infile.Gets();
  if (ptr == 0) {
    ErrorMsg("Getting number of frames from cpptraj failed.\n");
    return -1;
  }
  //Msg("DEBUG: '%s'\n", buffer);
  int nframes = 0;

  if (ptr[0] == 'F' && ptr[6] == ':') {
    std::string num = StringRoutines::NoTrailingWhitespace(std::string(ptr+8));
    //Msg("DEBUG: num=%s\n", num.c_str());
    if (StringRoutines::validInteger(num))
      nframes = StringRoutines::convertToInteger(num);
  }
  return nframes;
}
