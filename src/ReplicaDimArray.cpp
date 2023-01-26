#include "ReplicaDimArray.h"
#include "ReplicaDimension.h"

/** CONSTRUCTOR */
ReplicaDimArray::ReplicaDimArray() {}

/** Copy constructor */
ReplicaDimArray::ReplicaDimArray( ReplicaDimArray const& rhs) {
  for (DimArray::const_iterator it = rhs.Dims_.begin(); it != rhs.Dims_.end(); ++it)
    Dims_.push_back( (*it)->Copy() );
}

/** Clear all dims */
void ReplicaDimArray::ClearDims() {
  for (DimArray::const_iterator it = Dims_.begin(); it != Dims_.end(); ++it)
    delete *it;
}

/** Assignment */
ReplicaDimArray& ReplicaDimArray::operator=(ReplicaDimArray const& rhs) {
  if (&rhs == this) return *this;
  ClearDims();
  for (DimArray::const_iterator it = rhs.Dims_.begin(); it != rhs.Dims_.end(); ++it)
    Dims_.push_back( (*it)->Copy() );
  return *this;
}

/** DESTRUCTOR */
ReplicaDimArray::~ReplicaDimArray() {
  ClearDims();
}
