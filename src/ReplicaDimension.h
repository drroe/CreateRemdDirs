#ifndef INC_REPLICADIMENSION_H
#define INC_REPLICADIMENSION_H
#include <string>
#include <vector>
/// Abstract base class for replica dimension type.
class ReplicaDimension {
  public:
    typedef std::vector<std::string> Sarray;
    typedef std::vector<double> Darray;
    /// Should correspond to typeString_
    enum DimType { TEMP=0, TOPOLOGY, AMD_DIHEDRAL, SGLD, PH, NDIMTYPES };
    /// Should correspond to exchString_
    enum ExchType { TREMD=0, HREMD, PHREMD, NEXCHTYPES };
    /// CONSTRUCTOR - take dimension type and exchange type
    ReplicaDimension(DimType t, ExchType e) : type_(t), etype_(e) {}
    /// DESTRUCTOR - virtual since inherited
    virtual ~ReplicaDimension() {}
    // ---------------------------------
    /// \return Copy of this replica dimension
    virtual ReplicaDimension* Copy() const = 0;
    /// \return Replica dimension size.
    virtual unsigned int Size() const = 0;
    /// Read dimension file
    virtual int LoadDim(std::string const&) = 0;
    /// \return Dimension name.
    virtual const char* name() const = 0;
    // ---------------------------------
    /// \return Replica dimension type.
    DimType Type()            const { return type_;     }
    /// \return Replica description.
    const char* description() const { return description_.c_str(); }
    /// \return Dimension type description
    const char* type_str()    const { return typeString_[type_]; }
    /// \return Exchange type description.
    const char* exch_type()   const { return exchString_[etype_]; } 
  protected:
    // Set dimension description
    void SetDescription(std::string const& s) { description_ = s; }
  private:
    static const std::string emptystring_;
    static const char* exchString_[]; ///< Strings corresponding to ExchType
    static const char* typeString_[]; ///< Strings corresponding to DimType
    DimType type_;                    ///< Replica dimension type.
    std::string description_;         ///< Replica dimension description.
    ExchType etype_;                  ///< Replica exchange type
};
// -----------------------------------------------
/// Temperature dimension
class TemperatureDim : public ReplicaDimension {
  public:
    TemperatureDim() : ReplicaDimension(TEMP, TREMD) {}
    static ReplicaDimension* Alloc() { return (ReplicaDimension*)new TemperatureDim(); }
    unsigned int Size()     const { return temps_.size(); }
    double Temp0(int i)     const { return temps_[i]; }
    const char* name()      const { return "TREMD"; }
    int LoadDim(std::string const&);
    ReplicaDimension* Copy() const { return (ReplicaDimension*)new TemperatureDim(*this); }
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
    double SolvPH(int i)    const { return phs_[i]; }
    const char* name()      const { return "PHREMD"; }
    int LoadDim(std::string const&);
    ReplicaDimension* Copy() const { return (ReplicaDimension*)new PhDim(*this); }
  private:
    Darray phs_; ///< List of replica pHs.
};

// -----------------------------------------------
/// Topology dimension // FIXME need to rework temps into diagonal somehow
class TopologyDim : public ReplicaDimension {
  public:
    TopologyDim() : ReplicaDimension(TOPOLOGY, HREMD) {}
    static ReplicaDimension* Alloc() { return (ReplicaDimension*)new TopologyDim(); }
    unsigned int Size()               const { return tops_.size(); }
    std::string const& TopName(int i) const { return tops_[i]; }
    double Temp0(int i)               const { return temps_[i]; }
    const char* name()                const { return "HREMD"; }
    int LoadDim(std::string const&);
    ReplicaDimension* Copy() const { return (ReplicaDimension*)new TopologyDim(*this); }
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
    const char* name()      const { return "AMDHREMD"; }
    int LoadDim(std::string const&);
    ReplicaDimension* Copy() const { return (ReplicaDimension*)new AmdDihedralDim(*this); }

    /// \return Specified AMD alpha value
    double Alpha(int idx)   const { return d_alpha_[idx]; }
    /// \return Specified AMD threshhold value
    double Ethresh(int idx) const { return d_thresh_[idx]; }
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
    const char* name()      const { return "RXSGLD"; }
    int LoadDim(std::string const&);
    ReplicaDimension* Copy() const { return (ReplicaDimension*)new SgldDim(*this); }

    /// \return Specified SGLD temperature
    double SgTemp(int idx) const { return sgtemps_[idx]; }
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
