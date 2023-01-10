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

  // Get keywords
  int beg, end;
  if (args.GetKeyInteger(beg, "beg", -1)) return ERR;
  if (args.GetKeyInteger(end, "end", -1)) return ERR;
  std::string crd_dir = args.GetKey("crd");
  bool needsMdin = true; // TODO needed as an option?

  // Setup run
  if (activeSystem.SetupRunCreator( crd_dir, needsMdin )) {
    ErrorMsg("Run creation setup failed.\n");
    return ERR;
  }
  activeSystem.RunCreatorInfo();

  return OK;
}
