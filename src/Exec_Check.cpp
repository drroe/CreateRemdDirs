#include "Exec_Check.h"
#include "Messages.h"
#include "Manager.h"
#include "Cols.h"

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

  bool is_ok = true;

  if (args.HasKey("all")) {
    int pidx = 0;
    for (Manager::ProjectArray::const_iterator project = manager.Projects().begin();
                                               project != manager.Projects().end();
                                             ++project, ++pidx)
    {
      // Just in case we need to update the project
      //Project& modProject = manager.Set_Project(pidx);
      int sidx = 0;
      for (Project::SystemArray::const_iterator system = project->Systems().begin();
                                                system != project->Systems().end();
                                              ++system, ++sidx)
      {
        if (!system->CheckAllOptions()) {
          is_ok = false;
          Msg("** Issues detected for project %i system %i **\n\n", pidx, sidx);
        } else {
          Msg("** Project %i system %i seems OK.\n", pidx, sidx);
        }
      }
    }
  } else {
    // Get active system
    System& activeSystem = manager.ActiveProjectSystem();

    Msg("Project %i system %i: ", manager.ActiveProjectIdx(), manager.ActiveProjectSystemIdx());
    activeSystem.PrintSummary();

    is_ok = activeSystem.CheckAllOptions();
  }

  if (is_ok)
    return OK;
  return ERR;
}
