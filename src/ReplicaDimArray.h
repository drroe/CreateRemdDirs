#ifndef INC_REPLICADIMARRAY_H
#define INC_REPLICADIMARRAY_H
#include <vector>
#include <string>
#include "ReplicaDimension.h" // ReplicaDimension::DimType
/// Array of pointers to ReplicaDimension
class ReplicaDimArray {
  public:
    /// CONSTRUCTOR
    ReplicaDimArray();
    /// COPY CONSTRUCTOR
    ReplicaDimArray(ReplicaDimArray const&);
    /// ASSIGNMENT
    ReplicaDimArray& operator=(ReplicaDimArray const&);
    /// DESTRUCTOR
    ~ReplicaDimArray();

    /// Clear all dims
    void ClearDims();
    /// Load a dimension from file
    int LoadDimension(std::string const&);
    /// \return Nth dimension
    ReplicaDimension const& operator[](int idx) const { return *(Dims_[idx]); }
    /// \return first dimension
    ReplicaDimension const& FirstDim() const { return *(Dims_.front()); }
    /// \return Specified dimension
    ReplicaDimension const& Dim(ReplicaDimension::DimType) const;
    /// \return True if specified dimension is present
    bool HasDim(ReplicaDimension::DimType) const;
    /// \return Index of specified dimension
    int DimIdx(ReplicaDimension::DimType) const;
    /// \return True if no dimensions
    bool Empty() const { return Dims_.empty(); }
    /// \return Number of dimensions
    unsigned int Ndims() const { return Dims_.size(); }
  private:
    typedef std::vector<ReplicaDimension*> DimArray;
    DimArray Dims_;
    std::vector<int> idxs_; ///< Indices corresponding to ReplicaDimension::DimType
};
#endif
