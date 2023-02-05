#include "Exec_Create.h"
#include "Messages.h"
#include "Manager.h"
#include "Cols.h"

using namespace Messages;

Exec_Create::Exec_Create() {}

void Exec_Create::Help() const {
  Msg("[beg <#>] [{end <#>|nruns <#>}] [overwrite]\n");
}

Exec::RetType Exec_Create::Execute(Manager& manager, Cols& args) const {
  // Get the active system
  if (manager.NoProjects()) {
    ErrorMsg("No projects or systems, cannot create runs.\n");
    return ERR;
  }
  if (!manager.HasActiveProjectSystem()) {
    ErrorMsg("No active system, cannot create runs.\n");
    return ERR;
  }
  System& activeSystem = manager.ActiveProjectSystem();
  activeSystem.SetDebug( manager.Debug() );
  activeSystem.PrintSummary();

  // Get keywords
  int start_run, stop_run, nruns;
  if (args.GetKeyInteger(start_run, "beg", -1)) return ERR;
  if (args.GetKeyInteger(stop_run, "end", -1)) return ERR;
  if (args.GetKeyInteger(nruns, "nruns", -1)) return ERR;
  bool overwrite = args.HasKey("overwrite");

  // Set defaults if needed
  // Start
  if (start_run == -1) {
    // Start at the next available run
    if (activeSystem.Runs().empty())
      start_run = 0;
    else
      start_run = activeSystem.Runs().back().RunIndex() + 1;
  }
  // Stop
  if (stop_run == -1) {
    if (nruns > 0)
      stop_run = start_run + nruns - 1;
    else
      stop_run = start_run;
  }
  // Stop must be >= start
  if (stop_run < start_run) {
    ErrorMsg("STOP_RUN < START_RUN\n");
    return ERR;
  }
  // Determine total # runs
  nruns = stop_run - start_run + 1;

  // Check options
  if (start_run < 0 ) {
    ErrorMsg("Negative value for START_RUN\n");
    return ERR;
  }

  if (activeSystem.CreateRunDirectories(start_run, nruns, overwrite)) {
    ErrorMsg("Run directory creation failed.\n");
    return ERR;
  }
  
  return OK;
}
