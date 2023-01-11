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

  if (tgtProjectIdx < 0 || tgtSystemIdx < 0) {
    ErrorMsg("Must specify active project and system indices.\n");
    return ERR;
  }

  if ((unsigned int)tgtProjectIdx >= manager.Projects().size()) {
    ErrorMsg("Project index %i is out of range.\n", tgtProjectIdx);
    return ERR;
  }
  manager.SetActiveProject( tgtProjectIdx );
  Project& activeProject = manager.ActiveProject();

  if ((unsigned int)tgtSystemIdx >= activeProject.Systems().size()) {
    ErrorMsg("System index %i is out of range.\n", tgtSystemIdx);
    return ERR;
  }
  activeProject.SetActiveSystem( tgtSystemIdx );

  Msg("Project %i system %i is active.\n", tgtProjectIdx, tgtSystemIdx);
  System const& activeSystem = manager.ActiveProjectSystem();
  activeSystem.PrintInfo();

  return OK;
}
