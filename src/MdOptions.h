#ifndef INC_MDOPTIONS_H
#define INC_MDOPTIONS_H
#include "Option.h"
#include <string>
class MdOptions {
  public:
    MdOptions();

    Option<int>&    Set_N_Steps()      { return nsteps_; }
    Option<int>&    Set_RandomSeed()   { return random_seed_; }
    Option<int>&    Set_N_Exchanges()  { return nexchanges_; }
    Option<double>& Set_TimeStep()     { return timeStep_; }
    Option<double>& Set_Temperature0() { return temp0_; }
    Option<double>& Set_pH()           { return pH_; }

    Option<int>    const& N_Steps()      const { return nsteps_; }
    Option<int>    const& RandomSeed()   const { return random_seed_; }
    Option<int>    const& N_Exchanges()  const { return nexchanges_; }
    Option<double> const& TimeStep()     const { return timeStep_; }
    Option<double> const& Temperature0() const { return temp0_; }
    Option<double> const& pH()           const { return pH_; }
  private:
    Option<int> nsteps_;      ///< Number of simulation steps.
    Option<int> random_seed_; ///< Simulation random number generator seed.
    Option<int> nexchanges_;  ///< Number of exchange attempts.
    Option<double> timeStep_; ///< Simulation time step.
    Option<double> temp0_;    ///< Simulation bath temperature.
    Option<double> pH_;       ///< Simulation pH.
};
#endif
