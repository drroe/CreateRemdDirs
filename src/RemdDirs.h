#ifndef INC_REMDDIRS_H
#define INC_REMDDIRS_H
#include "ReplicaDimension.h"
#include "Groups.h"
class RemdDirs {
  public:
    static void OptHelp();
    RemdDirs();
    ~RemdDirs();
    int ReadOptions(std::string const&);
    int LoadDimensions();
    int CreateRun(int, int, std::string const&);
    int CreateMD(int, int, std::string const&);

    void SetDebug(int d) { debug_ = d; }
    void SetCrdDir(std::string const& c) { crd_dir_ = c; }
    std::string const& CrdFile() const { return crd_dir_; }
    const char* mdin_file() const { return mdin_file_.c_str(); }
    std::string const& Mdin_File() const { return mdin_file_; }
    int Nstlim() const { return nstlim_; }
    double Dt() const { return dt_; }
    int Numexchg() const { return numexchg_; }
    unsigned int Ndims() const { return DimFileNames_.size(); }
    std::string const& RunType() const { return run_type_; }
    /// \return Foremost topology for analysis/archiving
    std::string const& Topology() const {
      if (top_dim_ == -1) return top_file_;
      return Dims_[top_dim_]->TopName( 0 );
    }
    std::string const& TrajoutArgs() const { return trajoutargs_; }
    std::string const& FullArchive() const { return fullarchive_; }
  private:
    typedef std::vector<std::string> Sarray;
    Sarray DimFileNames_;

    std::string top_file_, trajoutargs_, fullarchive_, mdin_file_;
    int nstlim_, ig_, numexchg_;
    double dt_, temp0_;

    typedef std::vector<ReplicaDimension*> DimArray;
    DimArray Dims_;
    unsigned int totalReplicas_;
    int top_dim_; ///< Set to index of temp0 dim or -1 = global temp
    int temp0_dim_; ///< Set to index to topo dim or -1 = global topo
    int debug_;
    std::string run_type_;
    std::string additionalInput_;
    std::string crd_dir_; ///< Directory where input coordinates are.
    Groups groups_; ///< For setting up MREMD groups.
};
#endif
