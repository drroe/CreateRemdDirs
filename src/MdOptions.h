#ifndef INC_MDOPTIONS_H
#define INC_MDOPTIONS_H
#include "Option.h"
#include <string>
class MdOptions {
  public:
    MdOptions();

    Option<double> const& TimeStep() const { return timeStep_; }
  private:
    Option<int> nsteps_;      ///< Number of simulation steps.
    Option<int> random_seed_; ///< Simulation random number generator seed.
    Option<int> nexchanges_;  ///< Number of exchange attempts.
    Option<double> timeStep_; ///< Simulation time step.
    Option<double> temp0_;    ///< Simulation bath temperature.
};
#endif
