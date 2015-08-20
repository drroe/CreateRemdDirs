#ifndef INC_REMDDIRS_H
#define INC_REMDDIRS_H
#include "ReplicaDimension.h"
#include "Groups.h"
#include "FileRoutines.h" // StrArray
class RemdDirs {
  public:
    static void OptHelp();
    RemdDirs();
    ~RemdDirs();
    int ReadOptions(std::string const&);
    int Setup(std::string const&, bool);
    void Info() const;
    int CreateRuns(std::string const&, StrArray const&, int, bool);
    int CreateAnalyzeArchive(std::string const&, StrArray const&, int, int, bool, bool, bool);

    void SetDebug(int d) { debug_ = d; }
  private:
    enum RUNTYPE { MD=0, TREMD, HREMD, MREMD };

    std::string const& Topology() const {
      if (top_dim_ == -1) return top_file_;
      return Dims_[top_dim_]->TopName( 0 );
    }

    int LoadDimension(std::string const&);
    int CreateRemd(int, int, std::string const&);
    int CreateMD(int, int, std::string const&);
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
    int debug_;
    int n_md_runs_;               ///< Number of MD runs.
    int umbrella_;                ///< When > 0 indicates umbrella sampling write frequency.
    RUNTYPE runType_;             ///< Type of run from options file.
    std::string runDescription_;  ///< Run description
    std::string additionalInput_; ///< Hold any additional MDIN input.
    std::string crd_dir_;         ///< Directory where input coordinates are.
    Groups groups_;               ///< For setting up MREMD groups.
};
#endif
