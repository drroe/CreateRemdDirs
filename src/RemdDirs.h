#ifndef INC_REMDDIRS_H
#define INC_REMDDIRS_H
#include "Groups.h"
#include "FileRoutines.h" // StrArray
#include "MdinFile.h"
class ReplicaDimension;
/// Class responsible for creating run input and script
class RemdDirs {
  public:
    RemdDirs();
    ~RemdDirs();

    static void OptHelp();
    int ReadOptions(std::string const&,int);
    int Setup(std::string const&, bool);
    void Info() const;
    int CreateRuns(std::string const&, StrArray const&, int, bool);
    int CreateAnalyzeArchive(std::string const&, StrArray const&, int, int, bool, bool, bool, bool);

    void SetDebug(int d) { debug_ = d; }
  private:
    enum RUNTYPE { MD=0, TREMD, HREMD, PHREMD, MREMD };
    static const std::string groupfileName_;
    static const std::string remddimName_;
    /// \return Name of first topology file from the top_dim_ dimension.
    std::string const& Topology() const;

    int LoadDimension(std::string const&);
    std::string RefFileName(std::string const&) const;
    void WriteNamelist(TextFile&, std::string const&, MdinFile::TokenArray const&) const;
    int CreateRemd(int, int, std::string const&);
    int CreateMD(int, int, std::string const&);
    int WriteRunMD(std::string const&) const;
    int MakeMdinForMD(std::string const&, int, std::string const&, std::string const&) const;
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
    unsigned int totalReplicas_;  /// Total # of replicas based on dimensions
    int top_dim_;                 ///< Set to index of temp0 dim or -1 = global temp
    int temp0_dim_;               ///< Set to index to topo dim or -1 = global topo
    int ph_dim_;                  ///< Set to index of ph dim or -1 = no ph
    int debug_;
    int n_md_runs_;               ///< Number of MD runs.
    int umbrella_;                ///< When > 0 indicates umbrella sampling write frequency.
    bool override_irest_;         ///< If true do not set irest, use from MDIN
    bool override_ntx_;           ///< If true do not set ntx, use from MDIN
    bool uselog_;                 ///< If true use -l in groupfile
    bool crdDirSpecified_;        ///< If true, restart coords dir specified on command line.
    RUNTYPE runType_;             ///< Type of run from options file.
    std::string runDescription_;  ///< Run description
    std::string additionalInput_; ///< Hold any additional MDIN input.
    std::string crd_dir_;         ///< Directory where input coordinates are.
    std::string cpin_file_;       ///< CPIN file for constant pH
    std::string ref_file_;        ///< Reference file (MD) or path prefix (REMD)
    std::string ref_dir_;         ///< Directory where reference coords are (like crd_dir_)
    Groups groups_;               ///< For setting up MREMD groups.
};
#endif
