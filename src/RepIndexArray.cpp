#include "RepIndexArray.h"
#include "ReplicaDimension.h"
#include "StringRoutines.h"
#include "ReplicaDimArray.h"

/** CONSTRUCTOR - take replica dimension array. */ // TODO take increment_
RepIndexArray::RepIndexArray(ReplicaDimArray const& Dims) :
  indices_(Dims.Ndims(), 0),
  increment_(NORMAL),
  totalReplicas_(0)
{
  // Count total replicas
  if (!Dims.Empty()) {
    if (increment_ == NORMAL) {
      totalReplicas_ = 1;
      for (unsigned int idim = 0; idim != Dims.Ndims(); idim++)
        totalReplicas_ *= Dims[idim].Size();
    } else {
      // DIAGONAL
      totalReplicas_ = Dims[0].Size();
      for (unsigned int idim = 0; idim != Dims.Ndims(); idim++)
        if (Dims[idim].Size() > totalReplicas_)
          totalReplicas_ = Dims[idim].Size();
    }
  }
}

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
