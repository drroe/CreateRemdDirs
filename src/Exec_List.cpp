#include "Exec_List.h"
#include "Manager.h"
#include "Run.h"
#include "StringRoutines.h"
#include "Messages.h"
#include "Cols.h"

using namespace Messages;

Exec_List::Exec_List() {}

/** List all systems. */
Exec::RetType Exec_List::Execute(Manager& manager, Cols& args) const {
  int tgtidx = -1;
  std::string arg = args.GetKey("system");
  if (!arg.empty()) {
    if (StringRoutines::validInteger( arg )) {
      tgtidx = StringRoutines::convertToInteger( arg );
      //if (tgtidx < 0 || tgtidx >= manager.Systems().size()) {
      //  ErrorMsg("System index %i is out of range.\n", tgtidx);
      //  return ERR;
      //}
    } else {
      ErrorMsg("%s is not a valid system index.\n", arg.c_str());
      return ERR;
    }
  }

  int idx = 0;
  for (Manager::ProjectArray::const_iterator project = manager.Projects().begin();
                                             project != manager.Projects().end();
                                           ++project, ++idx)
  {
    int sidx = 0;
    Msg("Project: %s\n", project->name());
    for (Project::SystemArray::const_iterator system = project->Systems().begin();
                                              system != project->Systems().end();
                                            ++system, ++sidx)
    {
      if (tgtidx < 0) {
        Msg("  %i: ", sidx);
        system->PrintInfo();
      } else if (tgtidx == idx) {
        Msg("  %i: ", sidx);
        system->PrintInfo();
        for (System::RunArray::const_iterator run = system->Runs().begin();
                                              run != system->Runs().end(); ++run)
          (*run)->RunInfo();
        break;
      }
    }
  }
  return OK;
}
