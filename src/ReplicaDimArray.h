#ifndef INC_REPLICADIMARRAY_H
#define INC_REPLICADIMARRAY_H
#include <vector>
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

  private:
    typedef std::vector<ReplicaDimension*> DimArray;
    DimArray Dims_;
};
#endif
