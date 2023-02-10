#include "Exec_Info.h"
#include "Messages.h"
#include "Manager.h"

using namespace Messages;

/** CONSTRUCTOR */
Exec_Info::Exec_Info() {}

/** Help text */
void Exec_Info::Help() const {
}

/** <Command description goes here.> */
Exec::RetType Exec_Info::Execute(Manager& manager, Cols& args) const {
  // Ensure there is an active system
  if (!manager.HasActiveProjectSystem()) {
    ErrorMsg("No active system present.\n");
    return ERR;
  }
  // Get active system
  System& activeSystem = manager.ActiveProjectSystem();
  Msg("Project %i system %i: ", manager.ActiveProjectIdx(), manager.ActiveProjectSystemIdx());
  activeSystem.PrintSummary();

  activeSystem.PrintInfo();
  return OK;
}
