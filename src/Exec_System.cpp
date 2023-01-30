#include "Exec_System.h"
#include "Messages.h"
#include "Cols.h"
#include <cstdlib>

using namespace Messages;

/** CONSTRUCTOR */
Exec_System::Exec_System() {}

/** Help text */
void Exec_System::Help() const {
}

/** <Command description goes here.> */
Exec::RetType Exec_System::Execute(Manager& manager, Cols& args) const {
  if (args.Ncolumns() < 1) return ERR;
  // Put everything into a single string
  std::string cmd;
  for (Cols::const_iterator it = args.begin(); it != args.end(); ++it) {
    if (cmd.empty())
      cmd = *it;
    else
      cmd.append(" " + *it);
  }
  int err = system( cmd.c_str() );
  if (err != 0) Msg("Warning: '%s' returned %i\n", cmd.c_str(), err);
  return OK;
}
