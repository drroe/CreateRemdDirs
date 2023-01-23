#include "MdSetup.h"
#include "RepIndexArray.h"
#include "ReplicaDimension.h"
#include "Messages.h"
#include "FileRoutines.h"

using namespace Messages;

/** CONSTRUCTOR */
MdSetup::MdSetup() :
  temp0_dim_(-1),
  ph_dim_(-1)
{}

/** \return Temperature at specified index in temperature dim, or MD temperature if no dim. */
double MdSetup::Temperature(RepIndexArray const& Indices) const {
  if (temp0_dim_ == -1) return mdopts_.Temperature0().Val();
  return Dims_[temp0_dim_]->Temp0( Indices[temp0_dim_] );
}

/** Create input file for MD.
  * \param fname Name of MDIN file.
  * \param run_num Run number.
  * \param EXT Numerical extension (group #) corresponding to this MDIN when multiple MDINs needed.
  * \param Indices Array of replica indices if REMD
  * \param rep Overall replica index if REMD
  */
int MdSetup::MakeMdinForMD(MdOptions& currentMdOpts, int run_num, std::string const& EXT, 
                                 RepIndexArray const& Indices)
const
{
  using namespace FileRoutines;
  // Create input
  currentMdOpts = mdopts_;
  currentMdOpts.Set_Temperature0().SetVal( Temperature( Indices ) );
  if (ph_dim_ != -1)
    currentMdOpts.Set_pH().SetVal( Dims_[ph_dim_]->SolvPH( Indices[ph_dim_] ));
  if (mdopts_.RstFilename().IsSet()) {
    // Restraints
    std::string rf_name = add_path_prefix(mdopts_.RstFilename().Val() + EXT);
    // Ensure restraint file exists if specified.
    if (!fileExists( rf_name )) {
      ErrorMsg("Restraint file '%s' not found. Must specify absolute path"
               " or path relative to system dir.\n", rf_name.c_str());
      return 1;
    }
    currentMdOpts.Set_RstFilename().SetVal( rf_name );
    if (mdopts_.RstWriteFreq().IsSet() &&
        mdopts_.RstWriteFreq().Val() > 0)
    {
      if (EXT.empty())
        currentMdOpts.Set_RstWriteFile().SetVal("dumpave"); // FIXME allow user to customize
      else
        currentMdOpts.Set_RstWriteFile().SetVal("dumpave." + EXT);
    }
  }
  // Dimension-specific options
  for (unsigned int id = 0; id != Dims_.size(); id++) {
    if (Dims_[id]->Type() == ReplicaDimension::AMD_DIHEDRAL) { // TODO check for multiple amd dims?
      currentMdOpts.Set_AmdBoost().SetVal( MdOptions::AMD_TORSIONS );
      AmdDihedralDim const& amd = static_cast<AmdDihedralDim const&>( *(Dims_[id]) );
      currentMdOpts.Set_AmdEthresh().SetVal( amd.Ethresh()[ Indices[id] ] );
      currentMdOpts.Set_AmdAlpha().SetVal( amd.Alpha()[ Indices[id] ] );
    } else if (Dims_[id]->Type() == ReplicaDimension::ReplicaDimension::SGLD) { // TODO check for multiple sgld dims?
      currentMdOpts.Set_Sgld().SetVal( MdOptions::SGLD );
      SgldDim const& sgld = static_cast<SgldDim const&>( *(Dims_[id]) );
      currentMdOpts.Set_SgldTemp().SetVal( sgld.SgTemps()[ Indices[id] ] );
      // FIXME put in sgld dim
      double sgldAvgTime = 0.2;
      Msg("Warning: Using default SGLD avg time of %f\n", sgldAvgTime);
      currentMdOpts.Set_SgldAvgTime().SetVal( sgldAvgTime );
    }
  }
  return 0;
}
