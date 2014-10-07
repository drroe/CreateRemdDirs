#ifndef INC_GROUPS_H
#define INC_GROUPS_H
#include <vector>
#include <map>
#include "TextFile.h"
/// Class for setting up MREMD groups.
class Groups {
  public:
    typedef std::vector<unsigned int> Iarray;
    Groups() {}
    /// Prepare Groups for given # of dimensions.
    int SetupGroups(unsigned int);
    /// Add replica with given indices to its proper group in each dim.
    int AddReplica(Iarray const&, unsigned int);
    /// Print groups to screen
    void PrintGroups() const;
    /// Print groups for dimension to remd.dim file
    void WriteRemdDim(TextFile&, unsigned int, const char*, const char*) const;
    /// \return true if not yet set up
    bool Empty() const { return dimGroups_.empty(); }
  private:
    static inline void PrintArray(Iarray const&); 
    /** Map indices to group, 1 per dim. First Iarray is Ndims-1 (all
      * dimensions not including that dim), second is size of that dim.
      */
    typedef std::map<Iarray, Iarray> GroupArrayType;
    /// Hold GroupArrayType for each dimension.
    typedef std::vector<GroupArrayType> DimGroupType;
    DimGroupType dimGroups_;
};
#endif
