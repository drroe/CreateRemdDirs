#include "Exec_Save.h"
#include "Messages.h"
#include "Manager.h"

using namespace Messages;

/** CONSTRUCTOR */
Exec_Save::Exec_Save() {}

/** Help text */
void Exec_Save::Help() const {
}

/** <Command description goes here.> */
Exec::RetType Exec_Save::Execute(Manager& manager, Cols& args) const {
  // Ensure there is an active system
  if (!manager.HasActiveProjectSystem()) {
    ErrorMsg("No active system present.\n");
    return ERR;
  }
  // Get active system
  System& activeSystem = manager.ActiveProjectSystem();

  if (activeSystem.WriteSystemOptions()) return ERR;

  return OK;
}
