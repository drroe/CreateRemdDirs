#include "MdOptions.h"

/** CONSTRUCTOR */
MdOptions::MdOptions() :
  nsteps_(0),
  traj_write_freq_(0),
  random_seed_(-1),
  nexchanges_(0),
  timeStep_(0.002),
  temp0_(300.0)
{}

/** \return Total expected number of steps based on nsteps_ and nexchanges_ */
int MdOptions::Total_Steps() const {
  int numexchg;
  if (!nexchanges_.IsSet() || nexchanges_.Val() < 1)
    numexchg = 1;
  return (nsteps_.Val() * numexchg);
}

/** \return Expected number of frames based on traj write freq. and # steps. */
int MdOptions::Expected_Frames() const {
  if (traj_write_freq_.IsSet() && traj_write_freq_.Val() > 0)
    return (Total_Steps() / traj_write_freq_.Val());
  else
    return 0;
}

/** \return Total expected time based on expected total steps and time step. */
double MdOptions::Total_Time() const {
  return (double)Total_Steps() * timeStep_.Val();
}
