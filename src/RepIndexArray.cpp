#include "RepIndexArray.h"
#include "ReplicaDimension.h"
#include "StringRoutines.h"

/** Increment the replica index array. */
void RepIndexArray::Increment(std::vector<ReplicaDimension*> const& Dims) {
  // Increment first (fastest growing) index.
  indices_[0]++;
  // Increment remaining indices if necessary.
  for (unsigned int id = 0; id != Dims.size() - 1; id++)
  {
    if (indices_[id] == Dims[id]->Size()) {
      indices_[id] = 0; // Set this index to zero.
      indices_[id+1]++; // Increment next index.
    }
  }
}

/** \return indices as a string. */
std::string RepIndexArray::IndicesStr(unsigned int offset) const {
  std::string out;
  for (Iarray::const_iterator count = indices_.begin(); count != indices_.end(); ++count)
    out.append(" " + StringRoutines::integerToString( *count + offset ));
  return out;
}
