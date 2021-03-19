#include "NC.h"
#ifdef HAS_NETCDF
# include <netcdf.h>
#endif
#include "Messages.h"

int NC::CheckErr(int ncerr) {
# ifdef HAS_NETCDF
  if ( ncerr != NC_NOERR ) {
    Messages::ErrorMsg("NetCDF: %s\n", nc_strerror(ncerr));
    return 1;
  }
  return 0;
# else
  Messages::ErrorMsg("NC::CheckErr: No NetCDF support.\n");
  return 1;
# endif
}

int NC::GetDimInfo(int ncid, const char* attribute, unsigned int& length) {
# ifdef HAS_NETCDF
  int dimID;
  size_t slength = 0;
  length = 0;
  // Get dimid 
  if ( CheckErr(nc_inq_dimid(ncid, attribute, &dimID)) ) {
    Messages::ErrorMsg("Getting dimID for attribute %s\n", attribute);
    return -1;
  }
  // get Dim length 
  if ( CheckErr(nc_inq_dimlen(ncid, dimID, &slength)) ) {
    Messages::ErrorMsg("Getting length for attribute %s\n",attribute);
    return -1;
  }
  length = (unsigned int)slength;
  return dimID;
# else
  Messages::ErrorMsg("NC::CheckErr: No NetCDF support.\n");
  return -1;
# endif
}
