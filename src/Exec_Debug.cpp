#include "Exec_Debug.h"
#include "Messages.h"
#include "StringRoutines.h"
#include "Cols.h"
#include "Manager.h"

using namespace Messages;
using namespace StringRoutines;

Exec_Debug::Exec_Debug() {}

void Exec_Debug::Help() const {
  Msg("  Change global debug level.\n");
}

Exec::RetType Exec_Debug::Execute(Manager& manager, Cols& args) const {
  std::string dbgstr = args.NextColumn();
  if (dbgstr.empty()) {
    Msg("Current debug level is %i\n", manager.Debug());
  } else {
    if (validInteger( dbgstr )) {
      Msg("Setting debug level to '%s'\n", dbgstr.c_str());
      manager.SetDebug( convertToInteger(dbgstr) );
    } else {
      ErrorMsg("'%s' is not an integer; cannot set debug level.\n", dbgstr.c_str());
      return ERR;
    }
  }
  return OK;
}
