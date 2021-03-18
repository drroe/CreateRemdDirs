#include "Groups.h"
#include "Messages.h"

using namespace Messages;

int Groups::SetupGroups(unsigned int Ndims) {
  dimGroups_.clear();
  dimGroups_.resize( Ndims );
  return 0;
}

void Groups::PrintArray(Iarray const& idxs) {
  for (Iarray::const_iterator it = idxs.begin(); it != idxs.end(); ++it)
    Msg("{ %u", *it);
  Msg(" }\n");
}

/** It is expected that the replica number passed in starts from 1. */
int Groups::AddReplica(Iarray const& Indices, unsigned int rep) {
  // Put replica in its place in each dimension.
  for (unsigned int dim = 0; dim != dimGroups_.size(); dim++)
  {
    Iarray idxs;
    idxs.reserve( dimGroups_.size()-1 );
    // Put indices not corresponding to this dimension in idxs.
    for (unsigned int idxDim = 0; idxDim != dimGroups_.size(); idxDim++)
      if (idxDim != dim)
        idxs.push_back( Indices[idxDim] );
    // Find the corresponding group
    GroupArrayType::iterator grp = dimGroups_[dim].find( idxs );
    if (grp == dimGroups_[dim].end()) { // New group
      //Msg("\tNew group for dim %u: rep %u:", dim, rep);
      //PrintArray(idxs);
      dimGroups_[dim].insert( std::pair<Iarray,Iarray>( idxs, Iarray(1, rep) ) );
    } else {
      //Msg("\tAdding to group for dim %u: rep %u:", dim, rep);
      //PrintArray(grp->first);
      grp->second.push_back( rep );
    }
  }
  return 0;
}

void Groups::PrintGroups() const {
  for (DimGroupType::const_iterator dim = dimGroups_.begin();
                                    dim != dimGroups_.end(); ++dim)
  {
    Msg("  Dim %u:\n", dim - dimGroups_.begin());
    unsigned int gidx = 0;
    for (GroupArrayType::const_iterator grp = dim->begin();
                                        grp != dim->end(); ++grp, ++gidx)
    {
      Msg("    Group %u: ", gidx);
      for (Iarray::const_iterator rep = grp->second.begin();
                                  rep != grp->second.end(); ++rep)
        Msg(" %u", *rep);
      Msg("\n");
    }
  }
}

void Groups::WriteRemdDim(TextFile& REMDDIM, unsigned int id, 
                          const char* exch_type, const char* desc) const
{
  REMDDIM.Printf("Dimension %u\n", id); // Title
  REMDDIM.Printf("&multirem\n   exch_type = '%s',\n", exch_type);
  unsigned int gidx = 1;
  for (GroupArrayType::const_iterator grp = dimGroups_[id].begin();
                                      grp != dimGroups_[id].end();
                                    ++grp, ++gidx)
  {
    REMDDIM.Printf("   group(%u,:) = ", gidx);
    for (Iarray::const_iterator rep = grp->second.begin();
                                rep != grp->second.end(); ++rep)
      REMDDIM.Printf("%u,", *rep);
    REMDDIM.Printf("\n");
  }
  REMDDIM.Printf("   desc = '%s'\n/\n", desc);
}
