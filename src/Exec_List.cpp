#include "Exec_List.h"
#include "Manager.h"
#include "Run.h"
#include "StringRoutines.h"
#include "Messages.h"
#include "Cols.h"

using namespace Messages;

Exec_List::Exec_List() {}

/** List all systems. */
Exec::RetType Exec_List::Execute(Manager& manager, Cols& args) const {
  int tgtidx = -1;
  std::string arg = args.GetKey("system");
  if (!arg.empty()) {
    if (StringRoutines::validInteger( arg )) {
      tgtidx = StringRoutines::convertToInteger( arg );
      if (tgtidx < 0 || tgtidx >= manager.Systems().size()) {
        ErrorMsg("System index %i is out of range.\n", tgtidx);
        return ERR;
      }
    } else {
      ErrorMsg("%s is not a valid system index.\n", arg.c_str());
      return ERR;
    }
  }

  int idx = 0;
  for (Manager::SystemArray::const_iterator it = manager.Systems().begin(); it != manager.Systems().end(); ++it, ++idx)
  {
    if (tgtidx < 0) {
      Msg("  %i: ", idx);
      it->PrintInfo();
    } else if (tgtidx == idx) {
      Msg("  %i: ", idx);
      it->PrintInfo();
      for (System::RunArray::const_iterator run = it->Runs().begin(); run != it->Runs().end(); ++run)
        (*run)->RunInfo();
      break;
    }
  }
  return OK;
}
