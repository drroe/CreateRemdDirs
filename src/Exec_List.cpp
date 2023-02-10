#include "Exec_List.h"
#include "Manager.h"
#include "Run.h"
#include "Messages.h"
#include "Cols.h"

using namespace Messages;

Exec_List::Exec_List() {}

void Exec_List::Help() const {
  Msg("\t[{proj[ects] | [project <idx>] [system <idx>] [all]| running}]\n"
      "  List active project/system. If 'proj' or 'projects' is specifed, list\n"
      "  all projects. If 'project' and/or 'system' specified, show only that\n"
      "  project/system; the 'all' keyword can be used to show all systems or\n"
      "  runs. If 'running' specified only list jobs running in queue.\n"
     );
}

/** List all systems. */
Exec::RetType Exec_List::Execute(Manager& manager, Cols& args) const {
  enum ListModeType { ACTIVE = 0, RUNNING, PROJECTS };
  // Process input
  ListModeType listMode = ACTIVE;
  // Check for running mode args
  if (args.HasKey("running")) {
    listMode = RUNNING;
  }
  // Check for projects mode args
  static const int SHOW_ALL = -1;
  static const int HIDE_ALL = -2;
  int tgtProjectIdx = HIDE_ALL;
  int tgtSystemIdx = HIDE_ALL;
  int tgtRunIdx = HIDE_ALL;
  if (args.HasKey("projects") || args.HasKey("proj")) {
    listMode = PROJECTS;
    tgtProjectIdx = SHOW_ALL;
  }
  // Project
  if (args.GetKeyInteger(tgtProjectIdx, "project", -1)) return ERR;
  // If a specific project was chosen, list all systems by default.
  if (tgtProjectIdx > -1) {
    listMode = PROJECTS;
    tgtSystemIdx = SHOW_ALL;
  } else if (manager.Projects().size() == 1)
    tgtSystemIdx = SHOW_ALL;
  // System
  if (args.GetKeyInteger(tgtSystemIdx, "system", tgtSystemIdx)) return ERR;
  // If a specific system was chosen, list all runs by default.
  if (tgtSystemIdx > -1) {
    listMode = PROJECTS;
    tgtRunIdx = SHOW_ALL;
    // If no project specified, choose the active project
    if (tgtProjectIdx < 0)
      tgtProjectIdx = manager.ActiveProjectIdx();
  }
  // If there is only 1 project and 1 system, list all runs by default.
  if (manager.Projects().size() == 1 && manager.Projects()[0].Systems().size() == 1)
    tgtRunIdx = SHOW_ALL;
  // If a project has been chosen and 1 system, list all runs by default.
  if (tgtProjectIdx > -1) {
    if (manager.Projects()[tgtProjectIdx].Systems().size() == 1)
      tgtRunIdx = SHOW_ALL;
  }
  // 'all' overrides everything else that is not already set to a specific index
  if (args.HasKey("all")) {
    listMode = PROJECTS;
    if (tgtProjectIdx < 0)
      tgtProjectIdx = SHOW_ALL;
    if (tgtSystemIdx < 0)
      tgtSystemIdx = SHOW_ALL;
    if (tgtRunIdx < 0)
      tgtRunIdx = SHOW_ALL;
  }

  // -----------------------------------
  if (listMode == RUNNING) {
    Msg("Running jobs:\n");
    // List only running jobs
    int pidx = 0;
    for (Manager::ProjectArray::const_iterator project = manager.Projects().begin();
                                               project != manager.Projects().end();
                                             ++project, ++pidx)
    {
      // Just in case we need to update the project
      Project& modProject = manager.Set_Project(pidx);
      int sidx = 0;
      for (Project::SystemArray::const_iterator system = project->Systems().begin();
                                                system != project->Systems().end();
                                              ++system, ++sidx)
      {
        // Just in case we have to update the system
        System& modSystem = modProject.Set_System(sidx);
        int ridx = 0;
        for (RunArray::const_iterator run = system->Runs().begin();
                                      run != system->Runs().end();
                                    ++run, ++ridx)
        {
          bool needs_refresh = true;
          if (run->Stat().CurrentStat() == RunStatus::IN_QUEUE) {
            // Check if the system is now running.
            modSystem.RefreshSpecifiedRun( ridx );
            needs_refresh = false;
          }
          if (run->Stat().CurrentStat() == RunStatus::IN_PROGRESS) {
            if (needs_refresh)
              modSystem.RefreshSpecifiedRun( ridx );
            Msg("Project %i: System %i: Run %i: ", pidx, sidx, run->RunIndex());
            run->RunSummary();
          }
        }
      }
    }
    return OK;
  }

  // -----------------------------------
  if (listMode == ACTIVE) {
    Msg("Active project/system:\n");
    if (manager.HasActiveProjectSystem()) {
      System& activeSystem = manager.ActiveProjectSystem();
      activeSystem.RefreshCurrentRuns(false);
      Msg("Project %i: System %i: ", manager.ActiveProjectIdx(), 
          manager.ActiveProjectSystemIdx());
      // Count # frames
      unsigned int total_frames = 0;
      for (RunArray::const_iterator run = activeSystem.Runs().begin();
                                    run != activeSystem.Runs().end();
                                  ++run)
      {
        //Msg("DEBUG0 %u\n", run->Stat().CurrentTrajFrames());
        total_frames += run->Stat().CurrentTrajFrames();
      }
      Msg(" (%u frames) ", total_frames);
      activeSystem.PrintSummary();
      int ridx = 0;
      for (RunArray::const_iterator run = activeSystem.Runs().begin();
                                    run != activeSystem.Runs().end();
                                  ++run, ++ridx)
      {
        Msg("    %i: ", run->RunIndex());
        run->RunSummary();
      }
    } else {
      ErrorMsg("No active system.\n");
      return ERR;
    }
    return OK;
  }

  // -----------------------------------
  // listMode_ == PROJECTS

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
        Msg("Project *%i: %s", pidx, project->name());
      else
        Msg("Project  %i: %s", pidx, project->name());
      if (tgtSystemIdx == HIDE_ALL)
        Msg(": %zu systems.", project->Systems().size());
      Msg("\n");
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
          int total_frames = modSystem.RefreshCurrentRuns(false);
          if (total_frames < 0)
            Msg("Warning: Could not refresh status of system runs.\n");
          if (project->ActiveSystemIdx() == sidx)
            Msg("  System *%i: ", sidx);
          else
            Msg("  System  %i: ", sidx);
          Msg(" (%i frames) ", total_frames);
          system->PrintSummary();
          int ridx = 0;
          for (RunArray::const_iterator run = system->Runs().begin();
                                                run != system->Runs().end();
                                              ++run, ++ridx)
          {
            //Msg("DEBUG1 %u\n", run->Stat().CurrentTrajFrames());
            //if (tgtRunIdx == SHOW_ALL || tgtRunIdx == ridx || is_active_system)
            if (tgtRunIdx == SHOW_ALL || tgtRunIdx == ridx)
            {
              Msg("    %i: ", run->RunIndex());
              run->RunSummary();
            }
          }
        } // END loop over systems
      }
    }
  }  // END loop over projects
  return OK;
}
