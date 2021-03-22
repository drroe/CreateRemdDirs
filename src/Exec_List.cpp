#include "Exec_List.h"
#include "Manager.h"
#include "Run.h"

Exec_List::Exec_List() {}

Exec::RetType Exec_List::Execute(Manager& manager, Cols& args) const {

  for (Manager::SystemArray::const_iterator it = manager.Systems().begin(); it != manager.Systems().end(); ++it)
  {
    it->PrintInfo();
    for (System::RunArray::const_iterator run = it->Runs().begin(); run != it->Runs().end(); ++run)
      (*run)->RunInfo();
  }
  return OK;
}
