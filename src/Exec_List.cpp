#include "Exec_List.h"
#include "Manager.h"
#include "Run.h"
#include "Messages.h"
#include "Cols.h"

using namespace Messages;

Exec_List::Exec_List() {}

void Exec_List::Help() const {
  Msg("\t[project <idx>] [system <idx>]\n"
      "  List projects/systems.\n");
}

/** List all systems. */
Exec::RetType Exec_List::Execute(Manager& manager, Cols& args) const {
  static const int SHOW_ALL = -1;
  static const int HIDE_ALL = -2;
  int tgtProjectIdx = SHOW_ALL;
  int tgtSystemIdx = HIDE_ALL;
  int tgtRunIdx = HIDE_ALL;

  if (args.GetKeyInteger(tgtProjectIdx, "project", -1)) return ERR;
  // If a specific project was chosen, list all systems by default.
  if (tgtProjectIdx > -1)
    tgtSystemIdx = SHOW_ALL;
  if (args.GetKeyInteger(tgtSystemIdx, "system", tgtSystemIdx)) return ERR;
  // If a specific project was chosen, list all runs by default.
  if (tgtSystemIdx > -1)
    tgtRunIdx = SHOW_ALL;
  // 'all' overrides everything else that is not already set to a specific index
  if (args.HasKey("all")) {
    if (tgtProjectIdx < 0)
      tgtProjectIdx = SHOW_ALL;
    if (tgtSystemIdx < 0)
      tgtSystemIdx = SHOW_ALL;
    if (tgtRunIdx < 0)
      tgtRunIdx = SHOW_ALL;
  }

  Msg("Manager top directory: %s\n", manager.topDirName());

  if (tgtProjectIdx > -1 && (unsigned int)tgtProjectIdx >= manager.Projects().size()) {
    ErrorMsg("Project index %i is out of range.\n", tgtProjectIdx);
    return ERR;
  }
  int pidx = 0;
  for (Manager::ProjectArray::const_iterator project = manager.Projects().begin();
                                             project != manager.Projects().end();
                                           ++project, ++pidx)
  {
    if (tgtProjectIdx == SHOW_ALL || tgtProjectIdx == pidx) {
      if (manager.ActiveProjectIdx() == pidx)
        Msg("Project %i: (*)%s\n", pidx, project->name());
      else
        Msg("Project %i: %s\n", pidx, project->name());
      int sidx = 0;
      for (Project::SystemArray::const_iterator system = project->Systems().begin();
                                                system != project->Systems().end();
                                              ++system, ++sidx)
      {
        if (tgtSystemIdx == SHOW_ALL || tgtSystemIdx == sidx) {
          if (project->ActiveSystemIdx() == sidx)
            Msg("  %i: (*)", sidx);
          else
            Msg("  %i: ", sidx);
          system->PrintInfo();
          int ridx = 0;
          for (System::RunArray::const_iterator run = system->Runs().begin();
                                                run != system->Runs().end();
                                              ++run, ++ridx)
          {
            if (tgtRunIdx == SHOW_ALL || tgtRunIdx == ridx) {
              (*run)->RunInfo();
            }
          }
        } // END loop over systems
      }
    }
  }  // END loop over projects
  return OK;
}
