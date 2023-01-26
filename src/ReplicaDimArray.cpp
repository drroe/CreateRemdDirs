#include "ReplicaDimArray.h"
#include "ReplicaDimension.h"
#include "TextFile.h"
#include "Messages.h"

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
  return 0;
}

