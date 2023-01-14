#include "Exec_Create.h"
#include "Messages.h"
#include "Manager.h"
#include "Cols.h"

using namespace Messages;

Exec_Create::Exec_Create() {}

void Exec_Create::Help() const {
  Msg("\n");
}

Exec::RetType Exec_Create::Execute(Manager& manager, Cols& args) const {
  // Get the active system
  if (manager.NoProjects()) {
    Msg("Warning: No projects or systems, cannot create runs.\n");
    return ERR;
  }
  System& activeSystem = manager.ActiveProjectSystem();
  activeSystem.SetDebug( manager.Debug() );
  activeSystem.PrintInfo();

  // Get keywords
  int start_run, stop_run;
  if (args.GetKeyInteger(start_run, "beg", -1)) return ERR;
  if (args.GetKeyInteger(stop_run, "end", -1)) return ERR;
  std::string crd_dir = args.GetKey("crd");
  bool overwrite = args.HasKey("overwrite");

  // Set defaults if needed
  if (start_run > -1 && stop_run == -1)
    stop_run = start_run;
  // Stop must be >= start
  if (stop_run < start_run) {
    ErrorMsg("STOP_RUN < START_RUN\n");
    return ERR;
  }
  // Determine total # runs
  int nruns = stop_run - start_run + 1;

  // Check options
  if (start_run < 0 ) {
    ErrorMsg("Negative value for START_RUN\n");
    return ERR;
  }

  if (activeSystem.CreateRunDirectories(crd_dir, start_run, nruns, overwrite)) {
    ErrorMsg("Run directory creation failed.\n");
    return ERR;
  }
  
  return OK;
}
