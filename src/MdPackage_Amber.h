#ifndef INC_MDPACKAGE_AMBER_H
#define INC_MDPACKAGE_AMBER_H
#include "MdPackage.h"
#include "MdinFile.h"
class TextFile;
/// Amber-specific options
class MdPackage_Amber : public MdPackage {
  public:
    /// Print help text to stdout
    static void OptHelp();

    /// CONSTRUCTOR
    MdPackage_Amber();
    /// COPY CONSTRUCTOR
    MdPackage_Amber(MdPackage_Amber const&);
    /// ASSIGNMENT
    MdPackage_Amber& operator=(MdPackage_Amber const&);

    /// \return Copy of this class
    MdPackage* Copy() const { return (MdPackage*)new MdPackage_Amber(*this); }
    /// Process amber-specific creator option
    int ParseCreatorOption(std::string const&, std::string const&);
    /// Check creator options
    int CheckCreatorOptions(Creator const&) const;
    /// Read amber-specific input (i.e. MDIN input) from file
    int ReadPackageInput(MdOptions&, std::string const&);
    /// Create amber-specific input files
    int CreateInputFiles(Creator const&, int, int, std::string const&, std::string const&) const;
    /// \return Information on an existing run from output files
    RunStatus RunCurrentStatus(std::vector<std::string> const&) const;
  private:
    static const std::string groupfileName_; ///< REMD group file name TODO option
    static const std::string remddimName_;   ///< REMD remd.dim file name TODO option

    /// Write given namelist to specified file
    void writeNamelist(TextFile&, std::string const&, MdinFile::TokenArray const&) const;
    /// Write Amber MDIN file
    int writeMdInputFile(std::string const&, MdOptions const&, std::string const&, int,
                         RepIndexArray const&, unsigned int) const;
    /// Create multi-group MD input
    int create_multimd_input(Creator const&, int, int, std::string const&, std::string const&) const;
    /// Create single MD input
    int create_singlemd_input(Creator const&, int, int, std::string const&, std::string const&) const;
    /// Create remd input
    int create_remd_input(Creator const&, int, int, std::string const&, std::string const&) const;
    /// Read info from an MDOUT file; gets top name
    int read_mdout(RunStatus&, std::string const&, std::string&) const;
    /// Read # frames from traj file
    int read_traj_nframes(RunStatus&, std::string const&, std::string const&) const;

    std::string additionalInput_; ///< Hold any additional MDIN input
    bool override_irest_;         ///< If true do not set irest, use from MDIN
    bool override_ntx_;           ///< If true do not set ntx, use from MDIN
    MdinFile mdinFile_;           ///< Used to read input from MDIN
    bool uselog_;                 ///< If true add -l log command line arg
    std::string cpin_file_;       ///< Constant pH input file name
};
#endif
