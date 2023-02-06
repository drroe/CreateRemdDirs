#include "RepIndexArray.h"
#include "ReplicaDimension.h"
#include "StringRoutines.h"
#include "ReplicaDimArray.h"

/** Increment the replica index array. */
void RepIndexArray::Increment(ReplicaDimArray const& Dims) {
  if (increment_ == NORMAL) {
    // Increment first (fastest growing) index.
    indices_[0]++;
    // Increment remaining indices if necessary.
    for (unsigned int id = 0; id != Dims.Ndims() - 1; id++)
    {
      if (indices_[id] == Dims[id].Size()) {
        indices_[id] = 0; // Set this index to zero.
        indices_[id+1]++; // Increment next index.
      }
    }
  } else {
    // DIAGONAL
    // Increment all indices.
    for (unsigned int id = 0; id != Dims.Ndims(); id++) {
      indices_[id]++;
      if (indices_[id] == Dims[id].Size())
        indices_[id] = 0;
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
