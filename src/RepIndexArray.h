#ifndef INC_REPINDEXARRAY_H
#define INC_REPINDEXARRAY_H
#include <vector>
class ReplicaDimension;
/// Used to hold and count replica indices in 1 or more dimensions.
class RepIndexArray {
  public:
    typedef std::vector<unsigned int> Iarray;
    /// CONSTRUCTOR - take number of dimensions
    RepIndexArray(unsigned int n) : indices_(n, 0) {}
    /// \return index array
    Iarray const& Indices() const { return indices_; }
    /// Increment array
    void Increment(std::vector<ReplicaDimension*> const&);
    /// \return Index in specified dimension
    unsigned int operator[](int idx) const { return indices_[idx]; }
  private:
    Iarray indices_;
};
#endif
