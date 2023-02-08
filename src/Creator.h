#ifndef INC_CREATOR_H
#define INC_CREATOR_H
#include <vector>
#include "MdOptions.h"
#include "ReplicaDimArray.h"
#include "OptArray.h"
class RepIndexArray;
class TextFile;
/// Class responsible for creating run input and script
class Creator {
  public:
    /// Specific type of run the Creator will make (set in Setup); SYNC WITH RUNTYPESTR_ TODO deprecate
    enum RUNTYPE { MD=0, TREMD, HREMD, PHREMD, MREMD };
    /// CONSTRUCTOR
    Creator();

    typedef std::vector<std::string> Sarray;
    /// Print help to stdout
    static void OptHelp();
   
    /// Write creation options to a file
    int WriteOptions(TextFile&) const; 
    /// Read creation options from a file
    int ReadOptions(std::string const&);
    /// Check creator based on current options.
    int CheckCreator() const;
    /// \return True if the Creator is set up to make regular MD runs
    bool IsSetupForMD() const { return (runType_ == MD); }
    /// Print info to stdout
    void Info() const;
    /// \return Run type that this has been set for
    RUNTYPE TypeOfRun() const { return runType_; }
    /// \return Run description
    std::string const& RunDescription() const { return runDescription_; }
    /// \return Current MD options
    MdOptions const& MdOpts() const { return mdopts_; }
    /// Set debug level
    void SetDebug(int d) { debug_ = d; }
    /// Parse a single option from input file
    int ParseFileOption(OptArray::OptPair const&);
    /// Set MD options; only overwrite current options if not set.
    int SetMdOptions(MdOptions const&);
    /// \return Package-specific options
    OptArray const& PackageOpts() const { return package_opts_; }
    /// \return MD-specific package input file name
    std::string const& MdinFileName() const { return mdin_file_; }
    /// \return True if MD-specific package input file needs to be read
    bool MdinNeedsRead() const { return mdin_needs_read_; }
    /// Set MD-specific package input file status to read
    void Set_MdinAsRead() { mdin_needs_read_ = false; }

    /// \return Debug level
    int Debug() const { return debug_; }
    /// \return Numerical prefix/extension based on number and expected max
    std::string NumericalExt(int, int) const;
    /// \return Replica dimension array
    ReplicaDimArray const& Dims() const { return Dims_; }

    /// Create MDIN file for MD/REMD
    int MakeMdinForMD(MdOptions&, std::string const&, RepIndexArray const&) const;
    /// Create MDIN file for MD
    int MakeMdinForMD(MdOptions&, std::string const&) const;
    /// Create Run script for MD
    int WriteRunMD(std::string const&) const;

    // ----- Run Options -------------------------
    /// \return Number of MD runs to be performed in a single run directory.
    int N_MD_Runs() const { return n_md_runs_; }
    /// \return True if reference should be previous run restart
    bool UsePrevRestartAsRef() const { return usePrevRestartAsRef_; }
    /// \return True if doing diagonal REMD
    bool RemdDiagonal() const { return remd_diagonal_; }
    // ----- File names --------------------------
    /// \return Name of first topology file from the top_dim_ dimension or MD top file.
    std::string TopologyName() const;
    /// \return Name of topology at specified index in top_dim_ dimension (or MD top file).
    std::string TopologyName(RepIndexArray const&) const;
    /// \return Name of input coordinates file/directory.
    std::string const& CrdDir() const { return crd_dir_; }
    /// \return Name of reference coordinates file/directory
    std::string const& RefDir() const { return ref_dir_; }
  private: 
    /// Load a REMD dimension from file
    int LoadDimension(std::string const&);

    static const char* RUNTYPESTR_[]; ///< KEEP IN SYNC WITH RUNTYPE

    // File and MDIN variables
    std::string top_file_;
    MdOptions mdopts_;            ///< Hold MD options

    ReplicaDimArray Dims_;        ///< Hold any replica dimensions
    int debug_;
    int n_md_runs_;               ///< Number of MD runs.
    int fileExtWidth_;            ///< Filename extension width
    bool mdin_needs_read_;        ///< If true, mdin_file_ needs to be read by MdPackage
    bool usePrevRestartAsRef_;    ///< If true, reference will be restart of previous run.
    bool remd_diagonal_;          ///< If true, increment all dimensions for each replica.
    RUNTYPE runType_;             ///< Type of run from options file. TODO deprecate
    std::string runDescription_;  ///< Run description
    std::string crd_dir_;         ///< Directory where input coordinates are/MD input coords (from options file).
    std::string ref_dir_;         ///< Directory where reference coords are (like crd_dir_)
    std::string mdin_file_;       ///< Md package-specific input file
    Sarray dim_files_;            ///< REMD dimension files
    OptArray package_opts_;       ///< Hold any potential package-specific options
};
#endif
