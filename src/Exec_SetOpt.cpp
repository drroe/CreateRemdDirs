#include "Exec_SetOpt.h"
#include "Messages.h"

using namespace Messages;

/** CONSTRUCTOR */
Exec_SetOpt::Exec_SetOpt() {}

/** Help text */
void Exec_SetOpt::Help() const {
  Msg("\t{create|submit} <OPTION> <VALUE>}\n");
}

/** <Command description goes here.> */
Exec::RetType Exec_SetOpt::Execute(Manager& manager, Cols& args) const {
  return OK;
}
