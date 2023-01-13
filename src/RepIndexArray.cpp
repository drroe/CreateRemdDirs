#include "RepIndexArray.h"
#include "ReplicaDimension.h"

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
