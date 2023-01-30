#include "Exec_SetOpt.h"
#include "Messages.h"
#include "Manager.h"
#include "Cols.h"

using namespace Messages;

/** CONSTRUCTOR */
Exec_SetOpt::Exec_SetOpt() {}

/** Help text */
void Exec_SetOpt::Help() const {
  Msg("\t{create|submit} <OPTION> <VALUE>}\n");
}

/** <Command description goes here.> */
Exec::RetType Exec_SetOpt::Execute(Manager& manager, Cols& args) const {
  // Ensure there is a project
  if (manager.ActiveProjectIdx() < 0) {
    ErrorMsg("No projects present.\n");
    return ERR;
  }
  // Ensure project has a system
  Project& activeProject = manager.ActiveProject();
  if (activeProject.ActiveSystemIdx() < 0) {
    ErrorMsg("No systems present.\n");
    return ERR;
  }
  // Get active system
  System& activeSystem = manager.ActiveProjectSystem();
  // Get create or submit
  std::string TYPE = args.NextColumn();
  int create = -1;
  if (TYPE == "create")
    create = 1;
  else if (TYPE == "submit")
    create = 0;
  else {
    ErrorMsg("Expected 'create' or 'submit'.\n");
    return ERR;
  }
  // Get OPT
  std::string OPT = args.NextColumn();
  if (OPT.empty()) {
    ErrorMsg("No option specified.\n");
    return ERR;
  }
  // Everything else is VAR
  std::string arg = args.NextColumn();
  std::string VAR;
  while (!arg.empty()) {
    if (VAR.empty())
      VAR = arg;
    else
      VAR.append(" " + arg);
    arg = args.NextColumn();
  }
  if (VAR.empty()) {
    ErrorMsg("No variable(s) specified.\n");
    return ERR;
  }

  Msg("DEBUG: OPT=%s VAR=%s\n", OPT.c_str(), VAR.c_str());

  int ret;
  if (create == 1)
    ret = activeSystem.ParseOption( OPT, VAR );
  else
    Msg("FIXME: 'submit' not implemented yet.\n");
  if (ret == -1)
    return ERR;
  else if (ret == 0)
    Msg("Warning: Option not recognized.\n");

  return OK;
}
