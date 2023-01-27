#ifndef INC_REPLICADIMARRAY_H
#define INC_REPLICADIMARRAY_H
#include <vector>
#include <string>
class ReplicaDimension;
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

  private:
    typedef std::vector<ReplicaDimension*> DimArray;
    DimArray Dims_;
    std::vector<int> idxs_; ///< Indices corresponding to ReplicaDimension::DimType
};
#endif
