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
  else if (manager.Projects().size() == 1)
    tgtSystemIdx = SHOW_ALL;
  if (args.GetKeyInteger(tgtSystemIdx, "system", tgtSystemIdx)) return ERR;
  // If a specific project was chosen, list all runs by default.
  if (tgtSystemIdx > -1)
    tgtRunIdx = SHOW_ALL;
  // If there is only 1 project and 1 system, list all runs by default.
  if (manager.Projects().size() == 1 && manager.Projects()[0].Systems().size() == 1)
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
        Msg("Project *%i: %s\n", pidx, project->name());
      else
        Msg("Project  %i: %s\n", pidx, project->name());
      // Just in case we need to update the project
      Project& modProject = manager.Set_Project(pidx);
      int sidx = 0;
      for (Project::SystemArray::const_iterator system = project->Systems().begin();
                                                system != project->Systems().end();
                                              ++system, ++sidx)
      {
        //Msg("DEBUG: sidx=%i activeSystemIdx= %i\n", sidx, project->ActiveSystemIdx());
        if (tgtSystemIdx == SHOW_ALL || tgtSystemIdx == sidx) {
          // Ensure system is up to date (false = silent)
          System& modSystem = modProject.Set_System(sidx);
          modSystem.RefreshCurrentRuns(false);
          if (project->ActiveSystemIdx() == sidx)
            Msg("  System *%i: ", sidx);
          else
            Msg("  System  %i: ", sidx);
          // Count # frames
          unsigned int total_frames = 0;
          for (System::RunArray::const_iterator run = system->Runs().begin();
                                                run != system->Runs().end();
                                              ++run)
          {
            //Msg("DEBUG0 %u\n", run->Stat().CurrentTrajFrames());
            total_frames += run->Stat().CurrentTrajFrames();
          }
          Msg(" (%u frames) ", total_frames);
          system->PrintSummary();
          int ridx = 0;
          for (System::RunArray::const_iterator run = system->Runs().begin();
                                                run != system->Runs().end();
                                              ++run, ++ridx)
          {
            //Msg("DEBUG1 %u\n", run->Stat().CurrentTrajFrames());
            if (tgtRunIdx == SHOW_ALL || tgtRunIdx == ridx) {
              Msg("    %i: ", run->RunIndex());
              run->RunInfo();
            }
          }
        } // END loop over systems
      }
    }
  }  // END loop over projects
  return OK;
}
