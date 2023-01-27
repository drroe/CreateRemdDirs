#ifndef INC_CREATOR_H
#define INC_CREATOR_H
#include <vector>
#include "MdOptions.h"
#include "TextFile.h"
#include "ReplicaDimArray.h"
class RepIndexArray;
/// Class responsible for creating run input and script
class Creator {
  public:
    /// Specific type of run the Creator will make (set in Setup)
    enum RUNTYPE { MD=0, TREMD, HREMD, PHREMD, MREMD };
    /// CONSTRUCTOR
    Creator();

    typedef std::vector<std::string> Sarray;

    static void OptHelp();
    /// Read creation options from a file
    int ReadOptions(std::string const&);
    //int Setup(std::string const&, bool);
    /// \return True if the Creator is set up to make regular MD runs
    bool IsSetupForMD() const { return (runType_ == MD); }
    /// Print info to stdout
    void Info() const;
    /// \return Run type that this has been set for
    RUNTYPE TypeOfRun() const { return runType_; }
    /// \return MD-specific package input file name
    std::string const& MdinFileName() const { return mdin_file_; }
    /// \return Run description
    std::string const& RunDescription() const { return runDescription_; }
    //int CreateAnalyzeArchive(std::string const&, FileRoutines::StrArray const&, int, int, bool, bool, bool, bool);
    /// Set debug level
    void SetDebug(int d) { debug_ = d; }
    /// Set alternate coordinates location
    void SetSpecifiedCoords(std::string const& c) { specified_crd_ = c; }
    /// Set MD options
    int SetMdOptions(MdOptions const&);

    /// \return Debug level
    int Debug() const { return debug_; }

    /// \return Numerical prefix/extension based on number and expected max
    std::string NumericalExt(int, int) const;

    /// \return Replica dimension array
    ReplicaDimArray const& Dims() const { return Dims_; }

    /// \return Package-specific options
    TextFile::OptArray const& PackageOpts() const { return package_opts_; }

    /// Create MDIN file for REMD
    int MakeMdinForMD(MdOptions&, std::string const&, RepIndexArray const&) const;
    /// Create MDIN file for MD
    int MakeMdinForMD(MdOptions&, std::string const&) const;
    /// Create Run script for MD
    int WriteRunMD(std::string const&) const;

    // ----- Run Options -------------------------
    /// \return Number of MD runs to be performed in a single run directory.
    int N_MD_Runs() const { return n_md_runs_; }
    /// \return Umbrella write frequency; 0 means no umbrella
    //int UmbrellaWriteFreq() const { return umbrella_; }
    /// \retrurn Total number of replicas
    unsigned int TotalReplicas() const { return totalReplicas_; }
    // ----- File names --------------------------
    /// \return Name of first topology file from the top_dim_ dimension or MD top file.
    std::string TopologyName() const;
    /// \return Name of topology at specified index in top_dim_ dimension (or MD top file).
    std::string TopologyName(RepIndexArray const&) const;
    /// \return Temperature at specified index in temperature dim, or MD temperature if no dim.
    double Temperature(RepIndexArray const&) const;
    /// \return Array of input coordinate files given start run #, current run #, and prev. dir. name
    Sarray InputCoordsNames(int, int, std::string const&) const;
    /// \return Array of reference coordinate files
    Sarray RefCoordsNames() const;
    /// \return Group file name
    std::string const& GroupfileName() const { return groupfileName_; }
    /// \return CPIN file name
    std::string const& CPIN_Name() const { return cpin_file_; }
    /// \return REMD dimension name file
    std::string const& RemdDimName() const { return remddimName_; }
  private:

    static const std::string groupfileName_;
    static const std::string remddimName_;

    /// \return Array of input coords for multiple MD.
    Sarray inputCrds_multiple_md(std::string const&, std::string const&) const;
    /// \return Array containing single input coords for MD run.
    Sarray inputCrds_single_md(std::string const&, std::string const&) const;
    /// Perform internal setup, called by ReadOptions
    int setupCreator();

    int LoadDimension(std::string const&);
    std::string RefFileName(std::string const&) const; // TODO deprecate

    // File and MDIN variables
    std::string top_file_;
    std::string trajoutargs_;
    std::string fullarchive_;
    MdOptions mdopts_;            ///< Hold MD options

    ReplicaDimArray Dims_;        ///< Hold any replica dimensions
    unsigned int totalReplicas_;  ///< Total # of replicas based on dimensions
    int debug_;
    int n_md_runs_;               ///< Number of MD runs.
    int fileExtWidth_;            ///< Filename extension width
    RUNTYPE runType_;             ///< Type of run from options file.
    std::string runDescription_;  ///< Run description
    std::string additionalInput_; ///< Hold any additional MDIN input.
    std::string crd_dir_;         ///< Directory where input coordinates are/MD input coords (from options file).
    std::string specified_crd_;   ///< Input coords setet in Setup(); overrides crd_dir_.
    std::string crd_ext_;         ///< Input coords file name extension
    std::string cpin_file_;       ///< CPIN file for constant pH
    std::string ref_file_;        ///< Reference file (MD) or path prefix (REMD)
    std::string ref_dir_;         ///< Directory where reference coords are (like crd_dir_)
    std::string mdin_file_;       ///< Md package-specific input file
    TextFile::OptArray package_opts_; ///< Hold any potential package-specific options
};
#endif
