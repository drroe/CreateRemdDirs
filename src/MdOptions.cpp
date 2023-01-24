#include "MdOptions.h"

/** CONSTRUCTOR */
MdOptions::MdOptions() :
  nsteps_(0),
  random_seed_(-1),
  nexchanges_(0),
  timeStep_(0.002),
  temp0_(300.0)
{}

/** \return Expected number of frames based on traj write freq. and # steps. */
int MdOptions::Expected_Frames() const {
  if (nsteps_.IsSet() && nsteps_.Val() > 0) {
    int numexchg;
    if (!nexchanges_.IsSet() || nexchanges_.Val() < 1)
      numexchg = 1;
    else
      numexchg = nexchanges_.Val();
    if (traj_write_freq_.IsSet() && traj_write_freq_.Val() > 0)
      return (nsteps_.Val() * numexchg) / traj_write_freq_.Val();
  }
  return 0;
}
