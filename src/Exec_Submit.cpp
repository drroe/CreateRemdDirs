#include "Exec_Submit.h"
#include "Messages.h"
#include "Manager.h"
#include "Cols.h"

using namespace Messages;

Exec_Submit::Exec_Submit() {}

void Exec_Submit::Help() const {
  Msg("[beg <#>] [{end <#>|nruns <#>}] [overwrite] [test]\n");
}

Exec::RetType Exec_Submit::Execute(Manager& manager, Cols& args) const {
  // Get the active system
  if (manager.NoProjects()) {
    ErrorMsg("No projects or systems, cannot submit runs.\n");
    return ERR;
  }
  if (!manager.HasActiveProjectSystem()) {
    ErrorMsg("No active system, cannot submit runs.\n");
    return ERR;
  }
  System& activeSystem = manager.ActiveProjectSystem();
  activeSystem.SetDebug( manager.Debug() );
  activeSystem.PrintSummary();
  if (activeSystem.Runs().empty()) {
    ErrorMsg("System contains no runs to submit.\n");
    return ERR;
  }

  // Get keywords
  int start_run, stop_run, nruns;
  if (args.GetKeyInteger(start_run, "beg", -1)) return ERR;
  if (args.GetKeyInteger(stop_run, "end", -1)) return ERR;
  if (args.GetKeyInteger(nruns, "nruns", -1)) return ERR;
  bool overwrite = args.HasKey("overwrite");
  bool testOnly = args.HasKey("test");

  // Set defaults if needed
  // Start
  if (start_run == -1) {
    // Start at the first pending run
    for (RunArray::const_iterator it = activeSystem.Runs().begin();
                                  it != activeSystem.Runs().end(); ++it)
    {
      if (it->Stat().CurrentStat() == RunStatus::PENDING) {
        start_run = it->RunIndex();
        Msg("Starting at first pending run: %i\n", start_run);
        break;
      }
    }
    if (start_run == -1) {
      ErrorMsg("No pending runs.\n");
      return ERR;
    }
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
  // TODO previous job id
  if (activeSystem.SubmitRunDirectories(start_run, nruns, overwrite, "", testOnly)) {
    ErrorMsg("Run job submission failed.\n");
    return ERR;
  }
  
  return OK;
}
