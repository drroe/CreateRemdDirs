#include "Exec_List.h"
#include "Manager.h"

Exec_List::Exec_List() {}

Exec::RetType Exec_List::Execute(Manager& manager, Cols& args) const {

  manager.List();
  return OK;
}
