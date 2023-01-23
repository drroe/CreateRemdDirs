#ifndef INC_MDSETUP_H
#define INC_MDSETUP_H
#include <string>
#include <vector>
#include "MdOptions.h"
class RepIndexArray;
class ReplicaDimension;
/// Used to set up MD simulations
class MdSetup {
  public:
    /// CONSTRUCTOR
    MdSetup();

    typedef std::vector<ReplicaDimension*> DimArray;

    /// Create MdOptions for a run
    int MakeMdinForMD(MdOptions&, int, std::string const&, RepIndexArray const&) const;
  private:
    /// \return Temperature
    double Temperature(RepIndexArray const&) const;

    MdOptions mdopts_;  ///< Hold md options for creating new sims
    DimArray Dims_;     ///< Hold any replica dimensions

    int temp0_dim_;     ///< Set to index of temp. dim or -1 if no temp. dim
    int ph_dim_;        ///< Set to index of pH dim or -1 if no pH dim
};
#endif
