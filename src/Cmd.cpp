#include "Cmd.h"
#include "Exec.h"

/** Free the exec. */
void Cmd::Clear() {
  if (object_ != 0) delete object_;
}

bool Cmd::KeyMatches(std::string const& keyIn) const {
  for (key_iterator key = keywords_.begin(); key != keywords_.end(); ++key)
    if ( *key == keyIn ) return true;
  return false;
}
