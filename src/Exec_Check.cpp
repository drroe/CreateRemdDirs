#include "Exec_Check.h"
#include "Messages.h"
#include "Manager.h"

using namespace Messages;

/** CONSTRUCTOR */
Exec_Check::Exec_Check() {}

/** Help text */
void Exec_Check::Help() const {
}

/** <Command description goes here.> */
Exec::RetType Exec_Check::Execute(Manager& manager, Cols& args) const {
  // Ensure there is an active system
  if (!manager.HasActiveProjectSystem()) {
    ErrorMsg("No active system present.\n");
    return ERR;
  }
  // Get active system
  System& activeSystem = manager.ActiveProjectSystem();

  activeSystem.PrintSummary();

  bool is_ok = activeSystem.CheckAllOptions();
  if (is_ok)
    return OK;
  return ERR;
}
