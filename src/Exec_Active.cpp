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

  if (tgtProjectIdx == -1) {
    tgtProjectIdx = manager.ActiveProjectIdx();
    Msg("Using active project index %i\n", tgtProjectIdx);
  }

  if (manager.SetActiveProjectSystem(tgtProjectIdx, tgtSystemIdx)) return ERR;
 
  Project const& activeProject = manager.Projects()[manager.ActiveProjectIdx()];
  Msg("Project %i system %i is active.\n", manager.ActiveProjectIdx(),
      activeProject.ActiveSystemIdx());
  System const& activeSystem = manager.ActiveProjectSystem();
  activeSystem.PrintSummary();

  return OK;
}
