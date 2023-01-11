#include "Exec_Active.h"
#include "Messages.h"
#include "Cols.h"
#include "Manager.h"

using namespace Messages;

/** CONSTRUCTOR */
Exec_Active::Exec_Active() {}

/** Help text */
void Exec_Active::Help() const {
}

/** <Command description goes here.> */
Exec::RetType Exec_Active::Execute(Manager& manager, Cols& args) const {
  int tgtProjectIdx, tgtSystemIdx;
  if (args.GetKeyInteger(tgtProjectIdx, "project", -1)) return ERR;
  if (args.GetKeyInteger(tgtSystemIdx, "system", -1)) return ERR;

  if (manager.SetActiveProjectSystem(tgtProjectIdx, tgtSystemIdx)) return ERR;
  Msg("Project %i system %i is active.\n", tgtProjectIdx, tgtSystemIdx);
  System const& activeSystem = manager.ActiveProjectSystem();
  activeSystem.PrintInfo();

  return OK;
}