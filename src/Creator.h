#ifndef INC_CREATOR_H
#define INC_CREATOR_H
#include "Groups.h"
#include "MdinFile.h"
class ReplicaDimension;
/// Class responsible for creating run input and script
class Creator {
  public:
    /// Specific type of run the Creator will make (set in Setup)
    enum RUNTYPE { MD=0, TREMD, HREMD, PHREMD, MREMD };

    Creator();
    ~Creator();

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
    //int CreateRuns(std::string const&, FileRoutines::StrArray const&, int, bool);
    //int CreateAnalyzeArchive(std::string const&, FileRoutines::StrArray const&, int, int, bool, bool, bool, bool);
    /// Set debug level
    void SetDebug(int d) { debug_ = d; }
    /// Set alternate coordinates location
    void SetSpecifiedCoords(std::string const& c) { specified_crd_ = c; }

    /// \return Numerical prefix/extension based on number and expected max
    std::string NumericalExt(int, int) const;

    /// Create MDIN file for MD
    int MakeMdinForMD(std::string const&, int, std::string const&, std::string const&) const;
    /// Create Run script for MD
    int WriteRunMD(std::string const&) const;

    // ----- Run Options -------------------------
    /// \return Number of MD runs to be performed in a single run directory.
    int N_MD_Runs() const { return n_md_runs_; }
    /// \return Umbrella write frequency; 0 means no umbrella
    int UmbrellaWriteFreq() const { return umbrella_; }
    // ----- File names --------------------------
    /// \return Name of first topology file from the top_dim_ dimension or MD top file.
    std::string const& TopologyName() const;
    /// \return Array of input coordinate files based on run number
    Sarray InputCoordsNames(std::string const&, int, int) const;
    /// \return Array of reference coordinate files
    Sarray RefCoordsNames(std::string const&) const;
    /// \return Group file name
    std::string const& GroupfileName() const { return groupfileName_; }
    /// \return CPIN file name
    std::string const& CPIN_Name() const { return cpin_file_; }
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
    void WriteNamelist(TextFile&, std::string const&, MdinFile::TokenArray const&) const;
    //int CreateRemd(int, int, std::string const&);
    //int CreateMD(int, int, std::string const&);
    // File and MDIN variables
    std::string top_file_;
    std::string trajoutargs_;
    std::string fullarchive_;
    std::string mdin_file_;
    std::string rst_file_;
    int nstlim_;                  ///< Simulation number of steps/steps per exchange.
    int ig_;                      ///< Simulation random seed.
    int numexchg_;                ///< Simulation number of exchanges.
    double dt_;                   ///< Simulation time step.
    double temp0_;                ///< Simulation temperature.

    typedef std::vector<ReplicaDimension*> DimArray;
    MdinFile mdinFile_;           ///< Used to parse input from Amber MDIN file
    DimArray Dims_;               ///< Hold any replica dimensions
    unsigned int totalReplicas_;  ///< Total # of replicas based on dimensions
    int top_dim_;                 ///< Set to index of temp0 dim or -1 = global temp
    int temp0_dim_;               ///< Set to index to topo dim or -1 = global topo
    int ph_dim_;                  ///< Set to index of ph dim or -1 = no ph
    int debug_;
    int n_md_runs_;               ///< Number of MD runs.
    int umbrella_;                ///< When > 0 indicates umbrella sampling write frequency.
    int fileExtWidth_;            ///< Filename extension width
    bool override_irest_;         ///< If true do not set irest, use from MDIN
    bool override_ntx_;           ///< If true do not set ntx, use from MDIN
    bool uselog_;                 ///< If true use -l in groupfile
    bool crdDirSpecified_;        ///< If true, restart coords dir specified on command line. TODO deprecate
    RUNTYPE runType_;             ///< Type of run from options file.
    std::string runDescription_;  ///< Run description
    std::string additionalInput_; ///< Hold any additional MDIN input.
    std::string crd_dir_;         ///< Directory where input coordinates are/MD input coords (from options file).
    std::string specified_crd_;   ///< Input coords setet in Setup(); overrides crd_dir_.
    std::string crd_ext_;         ///< Input coords file name extension
    std::string cpin_file_;       ///< CPIN file for constant pH
    std::string ref_file_;        ///< Reference file (MD) or path prefix (REMD)
    std::string ref_dir_;         ///< Directory where reference coords are (like crd_dir_)
    Groups groups_;               ///< For setting up MREMD groups.
};
#endif
