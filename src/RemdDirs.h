#ifndef INC_REMDDIRS_H
#define INC_REMDDIRS_H
#include "ReplicaDimension.h"
#include "Groups.h"
#include "FileRoutines.h" // StrArray
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

    std::string const& Topology() const {
      if (top_dim_ == -1) return top_file_;
      return Dims_[top_dim_]->TopName( 0 );
    }

    int LoadDimension(std::string const&);
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
    int nstlim_, ig_, numexchg_;
    double dt_, temp0_;

    typedef std::vector<ReplicaDimension*> DimArray;
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
    Groups groups_;               ///< For setting up MREMD groups.
};
#endif
