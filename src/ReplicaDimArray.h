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
    int temp0_dim_; ///< Index of the temperature dimension
    int ph_dim_;    ///< Index of the pH dimension
    int top_dim_;   ///< Index of the topology dimension
};
#endif
