#ifndef INC_REPINDEXARRAY_H
#define INC_REPINDEXARRAY_H
#include <vector>
#include <string>
class ReplicaDimension;
class ReplicaDimArray;
/// Used to hold and count replica indices in 1 or more dimensions.
class RepIndexArray {
  public:
    enum IncrementType { NORMAL = 0, DIAGONAL };

    typedef std::vector<unsigned int> Iarray;
    /// CONSTRUCTOR - no indices
    RepIndexArray() : increment_(NORMAL) {}
    /// CONSTRUCTOR - take number of dimensions
    RepIndexArray(unsigned int n) : indices_(n, 0), increment_(NORMAL) {}
    /// \return index array
    Iarray const& Indices() const { return indices_; }
    /// \return true if no indices
    bool Empty() const { return indices_.empty(); }
    /// Increment array
    void Increment(ReplicaDimArray const&);
    /// \return Index in specified dimension
    unsigned int operator[](int idx) const { return indices_[idx]; }
/*    /// const iterator type
    typedef Iarray::const_iterator const_iterator;
    /// const iterator to beginning of indices
    const_iterator begin() const { return indices_.begin(); }
    /// const iterator to end of indices
    const_iterator end() const { return indices_.end(); }*/
    /// \return indices as a string, incremented by offset
    std::string IndicesStr(unsigned int) const;
  private:
    Iarray indices_;
    IncrementType increment_;
};
#endif
