#ifndef INC_MDOPTIONS_H
#define INC_MDOPTIONS_H
#include "Option.h"
#include <string>
class MdOptions {
  public:
    MdOptions();

    typedef Option<std::string> Sopt;

    /// AMD boost type
    enum AmdBoostType { AMD_NONE = 0, AMD_PE, AMD_TORSIONS, AMD_PE_TORSIONS };
    /// Self-guided langevin
    enum SgldType { SGLD_NONE = 0, SGLD };

    Option<int>&    Set_N_Steps()      { return nsteps_; }
    Option<int>&    Set_RandomSeed()   { return random_seed_; }
    Option<int>&    Set_N_Exchanges()  { return nexchanges_; }
    Option<double>& Set_TimeStep()     { return timeStep_; }
    Option<double>& Set_Temperature0() { return temp0_; }
    Option<double>& Set_pH()           { return pH_; }
    Sopt&           Set_RstFilename()  { return rst_file_; }
    Option<int>&    Set_RstWriteFreq() { return rstWriteFreq_; }
    Option<AmdBoostType>& Set_AmdBoost() { return amdBoost_; }
    Option<double>& Set_AmdEthresh()     { return amdEthresh_; }
    Option<double>& Set_AmdAlpha()       { return amdAlpha_; }
    Option<SgldType>& Set_Sgld()         { return sgld_; }
    Option<double>& Set_SgldAvgTime()    { return sgld_avgtime_; }
    Option<double>& Set_SgldTemp()    { return sgld_temp_; }

    Option<int>    const& N_Steps()      const { return nsteps_; }
    Option<int>    const& RandomSeed()   const { return random_seed_; }
    Option<int>    const& N_Exchanges()  const { return nexchanges_; }
    Option<double> const& TimeStep()     const { return timeStep_; }
    Option<double> const& Temperature0() const { return temp0_; }
    Option<double> const& pH()           const { return pH_; }
    Sopt           const& RstFilename()  const { return rst_file_; }
    Option<int>    const& RstWriteFreq() const { return rstWriteFreq_; }
    Option<AmdBoostType> const& AmdBoost() const { return amdBoost_; }
    Option<double> const& AmdEthresh()   const { return amdEthresh_; }
    Option<double> const& AmdAlpha()     const { return amdAlpha_; }
    Option<SgldType> const& Sgld()       const { return sgld_; }
    Option<double> const& SgldAvgTime()  const { return sgld_avgtime_; }
    Option<double> const& SgldTemp()     const { return sgld_temp_; }
  private:
    Option<int> nsteps_;       ///< Number of simulation steps.
    Option<int> random_seed_;  ///< Simulation random number generator seed.
    Option<int> nexchanges_;   ///< Number of exchange attempts.
    Option<double> timeStep_;  ///< Simulation time step.
    Option<double> temp0_;     ///< Simulation bath temperature.
    Option<double> pH_;        ///< Simulation pH.
    Sopt rst_file_;            ///< File holding restraint definitions
    Option<int> rstWriteFreq_; ///< Restraint value write frequency
    Option<AmdBoostType> amdBoost_; ///< Accelerated MD boost type
    Option<double> amdEthresh_; ///< Accelerated MD boost energy threshhold
    Option<double> amdAlpha_;   ///< Accelerated MD boost alpha
    Option<SgldType> sgld_;     ///< self-guided langevin type
    Option<double> sgld_avgtime_; ///< Self-guided langevin averaging time
    Option<double> sgld_temp_;    ///< Self-guided langevin temperature
};
#endif
