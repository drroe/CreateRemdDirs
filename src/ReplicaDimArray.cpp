#include "ReplicaDimArray.h"
#include "ReplicaDimension.h"

/** CONSTRUCTOR */
ReplicaDimArray::ReplicaDimArray() {}

/** Copy constructor */
ReplicaDimArray::ReplicaDimArray( ReplicaDimArray const& rhs) {
  for (DimArray::const_iterator it = rhs.Dims_.begin(); it != rhs.Dims_.end(); ++it)
    Dims_.push_back( (*it)->Copy() );
}

