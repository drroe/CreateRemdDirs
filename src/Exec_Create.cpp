#include "Exec_Create.h"
#include "Messages.h"
#include "Manager.h"
#include "Cols.h"
#include "StringRoutines.h"

using namespace Messages;
using namespace FileRoutines;

Exec_Create::Exec_Create() {}

void Exec_Create::Help() const {
  Msg("\n");
}

FileRoutines::StrArray Exec_Create::createRunDirs(int start_run, int stop_run, int digit_width)
{
  // Create array of run directories
  int runWidth = std::max( StringRoutines::DigitWidth(stop_run), digit_width );
  StrArray RunDirs;
  for (int run = start_run; run <= stop_run; ++run)
    RunDirs.push_back( "run." + StringRoutines::integerToString(run, runWidth) );
  return RunDirs;
}

Exec::RetType Exec_Create::Execute(Manager& manager, Cols& args) const {
  // Get the active system
  if (manager.NoProjects()) {
    Msg("Warning: No projects or systems, cannot create runs.\n");
    return ERR;
  }
  System& activeSystem = manager.ActiveProjectSystem();

  // Get keywords
  int start_run, stop_run;
  if (args.GetKeyInteger(start_run, "beg", -1)) return ERR;
  if (args.GetKeyInteger(stop_run, "end", -1)) return ERR;
  std::string crd_dir = args.GetKey("crd");
  bool needsMdin = true; // TODO needed as an option?
  bool overwrite = false;

  // Setup run
  if (activeSystem.SetupRunCreator( crd_dir, needsMdin )) {
    ErrorMsg("Run creation setup failed.\n");
    return ERR;
  }
  activeSystem.RunCreatorInfo();

  // Check options
  if (start_run < 0 ) {
    ErrorMsg("Negative value for START_RUN\n");
    return ERR;
  }
  if (stop_run < start_run) {
    ErrorMsg("STOP_RUN < START_RUN\n");
    return ERR;
  }

  //activeSystem.ChangeToSystemDir();
  // Change to top dir
  if (ChangeDir( manager.TopDirName() )) return ERR;
  if (activeSystem.Runs().empty()) {
    // No existing runs.
    StrArray RunDirs = createRunDirs(start_run, stop_run, 3);
    Msg("Creating %i runs from %i to %i\n", stop_run - start_run + 1, start_run, stop_run);
    if (activeSystem.CreateRuns(activeSystem.SystemDirName(), RunDirs,
                                start_run, overwrite)) return ERR;
  } else {
    Msg("NOT YET SET UP FOR HAVING EXISTING RUNS.\n");
  }

  return OK;
}
