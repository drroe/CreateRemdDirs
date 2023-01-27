#include "ReplicaDimArray.h"
#include "TextFile.h"
#include "Messages.h"

/** CONSTRUCTOR */
ReplicaDimArray::ReplicaDimArray() :
  idxs_((int)ReplicaDimension::NDIMTYPES, -1)
{}

/** Copy constructor */
ReplicaDimArray::ReplicaDimArray( ReplicaDimArray const& rhs) :
  idxs_(rhs.idxs_)
{
  for (DimArray::const_iterator it = rhs.Dims_.begin(); it != rhs.Dims_.end(); ++it)
    Dims_.push_back( (*it)->Copy() );
}

/** Clear all dims */
void ReplicaDimArray::ClearDims() {
  for (DimArray::const_iterator it = Dims_.begin(); it != Dims_.end(); ++it)
    delete *it;
  idxs_.assign((int)ReplicaDimension::NDIMTYPES, -1);
}

/** Assignment */
ReplicaDimArray& ReplicaDimArray::operator=(ReplicaDimArray const& rhs) {
  if (&rhs == this) return *this;
  ClearDims();
  idxs_ = rhs.idxs_;
  for (DimArray::const_iterator it = rhs.Dims_.begin(); it != rhs.Dims_.end(); ++it)
    Dims_.push_back( (*it)->Copy() );
  return *this;
}

/** DESTRUCTOR */
ReplicaDimArray::~ReplicaDimArray() {
  ClearDims();
}

/** Load a dimension from file. */
int ReplicaDimArray::LoadDimension(std::string const& dfile) {
  using namespace Messages;
  TextFile infile;
  // File existence already checked.
  if (infile.OpenRead(dfile)) return 1;
  // Determine dimension type from first line.
  std::string firstLine = infile.GetString();
  if (firstLine.empty()) {
    ErrorMsg("Could not read first line of dimension file '%s'\n", dfile.c_str());
    return 1;
  }
  infile.Close(); 
  // Allocate proper dimension type and load.
  ReplicaDimension* dim = ReplicaAllocator::Allocate( firstLine );
  if (dim == 0) {
    ErrorMsg("Unrecognized dimension type: %s\n", firstLine.c_str());
    return 2;
  }
  // Push it here so it will be deallocated if there is an error
  Dims_.push_back( dim ); 
  if (dim->LoadDim( dfile )) {
    ErrorMsg("Loading info from dimension file '%s'\n", dfile.c_str());
    return 1;
  }
  Msg("    Dim %u: %s (%u)\n", Dims_.size(), dim->description(), dim->Size());
  // Do some checking
  if (idxs_[dim->Type()] != -1) {
    ErrorMsg("At most one dimension that provides %s should be specified.\n", dim->type_str());
    return 1;
  }
  idxs_[dim->Type()] = (int)(Dims_.size() - 1);

  return 0;
}

/** \return True if specified dimension is present. */
bool ReplicaDimArray::HasDim(ReplicaDimension::DimType typeIn) const {
  return (idxs_[typeIn] > -1);
}

/** \return Specified dimension. */
ReplicaDimension const& ReplicaDimArray::Dim(ReplicaDimension::DimType typeIn) const {
  if (!HasDim(typeIn)) {
    Messages::ErrorMsg("ReplicaDimArray called with invalid type.");
  }
  return *(Dims_[ idxs_[typeIn] ]);
}

/** \return Index of specified dimension. */
int ReplicaDimArray::DimIdx(ReplicaDimension::DimType typeIn) const {
  return idxs_[typeIn];
}
