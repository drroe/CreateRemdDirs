#ifndef INC_MDPACKAGE_AMBER_H
#define INC_MDPACKAGE_AMBER_H
#include "MdPackage.h"
#include "MdinFile.h"
class TextFile;
/// Amber-specific options
class MdPackage_Amber : public MdPackage {
  public:
    /// CONSTRUCTOR
    MdPackage_Amber();
    /// COPY CONSTRUCTOR
    MdPackage_Amber(MdPackage_Amber const&);
    /// ASSIGNMENT
    MdPackage_Amber& operator=(MdPackage_Amber const&);

    /// \return Copy of this class
    MdPackage* Copy() const { return (MdPackage*)new MdPackage_Amber(*this); }
    /// Read amber-specific input options from file
    int ReadInputOptions(std::string const&);
    /// Create amber-specific input files
    int CreateInputFiles(Creator const&, int, int, std::string const&, std::string const&) const;

  private:
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

    std::string additionalInput_; ///< Hold any additional MDIN input
    bool override_irest_;         ///< If true do not set irest, use from MDIN
    bool override_ntx_;           ///< If true do not set ntx, use from MDIN
    MdinFile mdinFile_;           ///< Used to read input from MDIN
};
#endif
