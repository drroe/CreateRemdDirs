#include "Exec_SetOpt.h"
#include "Messages.h"
#include "Manager.h"
#include "Cols.h"

using namespace Messages;

/** CONSTRUCTOR */
Exec_SetOpt::Exec_SetOpt() {}

/** Basic help text. */
void Exec_SetOpt::Help() const {
  Msg("\t<OPTION> <VALUE>}\n");
}

/** Help text */
void Exec_SetOpt::Help(Cols& args) const {
  Help();
  if (args.Ncolumns() == 2) {
    Msg("\tType '%s %s create' or '%s %s submit' for create/submit options.\n",
        args[0].c_str(), args[1].c_str(), args[0].c_str(), args[1].c_str());
  } else {
    if (args.HasKey("create"))
      Creator::OptHelp();
    if (args.HasKey("submit"))
      Submitter::OptHelp();
  }
}

/** <Command description goes here.> */
Exec::RetType Exec_SetOpt::Execute(Manager& manager, Cols& args) const {
  // Ensure there is an active system
  if (!manager.HasActiveProjectSystem()) {
    ErrorMsg("No active system present.\n");
    return ERR;
  }
  // Get active system
  System& activeSystem = manager.ActiveProjectSystem();
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

  int ret = activeSystem.ParseOption( OPT, VAR );
  if (ret == -1) {
    ErrorMsg("Could not process '%s = %s'\n", OPT.c_str(), VAR.c_str());
    return ERR;
  } else if (ret == 0)
    Msg("Warning: Option '%s = %s' not recognized.\n", OPT.c_str(), VAR.c_str());

  return OK;
}
