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
  // Ensure there is a project
  if (manager.ActiveProjectIdx() < 0) {
    ErrorMsg("No projects present.\n");
    return ERR;
  }
  // Ensure project has a system
  Project& activeProject = manager.ActiveProject();
  if (activeProject.ActiveSystemIdx() < 0) {
    ErrorMsg("No systems present.\n");
    return ERR;
  }
  // Get active system
  System& activeSystem = manager.ActiveProjectSystem();

  activeSystem.PrintInfo();
  return OK;
}
