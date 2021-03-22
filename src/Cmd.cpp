#include "Cmd.h"
#include "Exec.h"

/** Free the exec. */
void Cmd::Clear() {
  if (object_ != 0) delete object_;
}
