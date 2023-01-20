#ifndef INC_REPLICADIMENSION_H
#define INC_REPLICADIMENSION_H
#include "TextFile.h"
/// Abstract base class for replica dimension type.
class ReplicaDimension {
  public:
    typedef std::vector<std::string> Sarray;
    typedef std::vector<double> Darray;
    enum DimType { NO_TYPE=0, TEMP, TOPOLOGY, AMD_DIHEDRAL, SGLD, PH };
    enum ExchType { NO_EXCH=0, TREMD, HREMD, PHREMD };
    ReplicaDimension() : type_(NO_TYPE), etype_(NO_EXCH) {}
    ReplicaDimension(DimType t, ExchType e) : type_(t), etype_(e) {}
    virtual ~ReplicaDimension() {}
    // ---------------------------------
    /// \return Replica dimension size.
    virtual unsigned int Size() const = 0;
    /// Read dimension file
    virtual int LoadDim(std::string const&) = 0;
    /// \return true if dimension provides bath temperatures (temp0).
    virtual bool ProvidesTemp0() const = 0;
    /// \return true if dimension provides ph TODO pure virtual
    virtual bool ProvidesPh() const { return false; }
    /// \return true if dimension provides topology files.
    virtual bool ProvidesTopFiles() const = 0;
    /// \return Dimension name.
    virtual const char* name() const = 0;
    /// Write relevant input to MDIN if necessary
    virtual int WriteMdin(int, TextFile&) const { return 0; }
    /// \return any necessary additions to the groupfile
    virtual std::string Groupline(std::string const&) const { return std::string(""); }
    /// \return output dir if necessary
    virtual const char* OutputDir() const { return 0; }
    /// \return Topology name
    virtual std::string const& TopName(int) const { return emptystring_; }
    /// \return Temperature
    virtual double Temp0(int)  const { return -1.0; }
    /// \return pH
    virtual double SolvPH(int) const { return -1.0; }
    // ---------------------------------
    /// \return Replica dimension type.
    DimType Type() const { return type_;     }
    /// \return Replica description.
    const char* description() const { return description_.c_str(); }
    /// \return Exchange type description.
    const char* exch_type() const { return exchString_[etype_]; } 
  protected:
    // Set dimension description
    void SetDescription(std::string const& s) { description_ = s; }
  private:
    static const std::string emptystring_;
    static const char* exchString_[];
    DimType type_; ///< Replica dimension type.
    std::string description_; ///< Replica dimension description.
    ExchType etype_; ///< Replica exchange type
};
// -----------------------------------------------
/// Temperature dimension
class TemperatureDim : public ReplicaDimension {
  public:
    TemperatureDim() : ReplicaDimension(TEMP, TREMD) {}
    static ReplicaDimension* Alloc() { return (ReplicaDimension*)new TemperatureDim(); }
    unsigned int Size()     const { return temps_.size(); }
    bool ProvidesTemp0()    const { return true; }
    bool ProvidesTopFiles() const { return false; }
    double Temp0(int i)     const { return temps_[i]; }
    const char* name()      const { return "TREMD"; }
    int LoadDim(std::string const&);
  private:
    Darray temps_; ///< List of replica temperatures.
};

// -----------------------------------------------
/// pH dimension TODO explicit vs implicit?
class PhDim : public ReplicaDimension {
  public:
    PhDim() : ReplicaDimension(PH, PHREMD) {}
    static ReplicaDimension* Alloc() { return (ReplicaDimension*)new PhDim(); }
    unsigned int Size()     const { return phs_.size(); }
    bool ProvidesTemp0()    const { return false; }
    bool ProvidesPh()       const { return true;  }
    bool ProvidesTopFiles() const { return false; }
    double SolvPH(int i)    const { return phs_[i]; }
    const char* name()      const { return "PHREMD"; }
    int LoadDim(std::string const&);
  private:
    Darray phs_; ///< List of replica pHs.
};

// -----------------------------------------------
/// Topology dimension
class TopologyDim : public ReplicaDimension {
  public:
    TopologyDim() : ReplicaDimension(TOPOLOGY, HREMD) {}
    static ReplicaDimension* Alloc() { return (ReplicaDimension*)new TopologyDim(); }
    unsigned int Size()               const { return tops_.size(); }
    bool ProvidesTemp0()              const { return !temps_.empty(); }
    bool ProvidesTopFiles()           const { return true; }
    std::string const& TopName(int i) const { return tops_[i]; }
    double Temp0(int i)               const { return temps_[i]; }
    const char* name()                const { return "HREMD"; }
    int LoadDim(std::string const&);
  private:
    Sarray tops_; ///< List of replica topologies
    Darray temps_; ///< Optional corresponding list of temperatures
}; 

// -----------------------------------------------
/// AMD dihedral boost dimension
class AmdDihedralDim : public ReplicaDimension {
  public:
    AmdDihedralDim() : ReplicaDimension(AMD_DIHEDRAL, HREMD) {}
    static ReplicaDimension* Alloc() { return (ReplicaDimension*)new AmdDihedralDim(); }
    unsigned int Size()     const { return d_alpha_.size(); }
    const char* OutputDir() const { return "AMD"; }
    bool ProvidesTemp0()    const { return false; }
    bool ProvidesTopFiles() const { return false; }
    const char* name()      const { return "AMDHREMD"; }
    int LoadDim(std::string const&);
    std::string Groupline(std::string const&) const;
    int WriteMdin(int, TextFile&) const;

    /// \return Array of AMD alpha values
    Darray const& Alpha() const { return d_alpha_; }
    /// \return Array of AMD threshhold values
    Darray const& Ethresh() const { return d_thresh_; }
  private:
    Darray d_alpha_; ///< List of amd dihedral alpha values
    Darray d_thresh_; ///< List of amd dihedral threshhold values
};

// -----------------------------------------------
/// RXSGLD dimension
class SgldDim : public ReplicaDimension {
  public:
    SgldDim() : ReplicaDimension(SGLD, TREMD) {}
    static ReplicaDimension* Alloc() { return (ReplicaDimension*)new SgldDim(); }
    unsigned int Size()     const { return sgtemps_.size(); }
    bool ProvidesTemp0()    const { return false; }
    bool ProvidesTopFiles() const { return false; }
    const char* name()      const { return "RXSGLD"; }
    int LoadDim(std::string const&);
    int WriteMdin(int, TextFile&) const;
  private:
    Darray sgtemps_; ///< Self-guided Langevin temperatures.
};

// -----------------------------------------------------------------------------
namespace ReplicaAllocator {
  typedef ReplicaDimension* (*AllocatorType)();

  struct Token {
    const char* Key;
    AllocatorType Alloc;
  };
  static const Token AllocArray[] = {
    { "#Temperature",  TemperatureDim::Alloc },
    { "#Temp",         TemperatureDim::Alloc },
    { "#Hamiltonian",  TopologyDim::Alloc    },
    { "#amd_dihedral", AmdDihedralDim::Alloc },
    { "#SGLD",         SgldDim::Alloc        },
    { "#PH",           PhDim::Alloc          },
    { 0,               0                     }
  };
  ReplicaDimension* Allocate(std::string const&);
}
#endif
