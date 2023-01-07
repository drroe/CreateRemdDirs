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
  int tgtProjectIdx = -1;
  int tgtSystemIdx = -1;

  if (args.GetKeyInteger(tgtProjectIdx, "project", -1)) return ERR;
  if (args.GetKeyInteger(tgtSystemIdx, "system", -1)) return ERR;
  if (args.HasKey("all")) tgtSystemIdx = -2;
  if (tgtProjectIdx != -1) tgtSystemIdx = -2;

  if (tgtProjectIdx > -1 && tgtProjectIdx >= manager.Projects().size()) {
    ErrorMsg("Project index %i is out of range.\n", tgtProjectIdx);
    return ERR;
  }

  int pidx = 0;
  for (Manager::ProjectArray::const_iterator project = manager.Projects().begin();
                                             project != manager.Projects().end();
                                           ++project, ++pidx)
  {
    if (tgtProjectIdx < 0 || tgtProjectIdx == pidx) {
      Msg("Project %i: %s\n", pidx, project->name());
      int sidx = 0;
      for (Project::SystemArray::const_iterator system = project->Systems().begin();
                                                system != project->Systems().end();
                                              ++system, ++sidx)
      {
        if (tgtSystemIdx == -2 || tgtSystemIdx == sidx) {
          Msg("  %i: ", sidx);
          system->PrintInfo();
          for (System::RunArray::const_iterator run = system->Runs().begin();
                                                run != system->Runs().end(); ++run)
            (*run)->RunInfo();
        }
      }
    }
  }
  return OK;
}
