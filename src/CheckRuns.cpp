#include <cmath>
#include <cstdlib>
#ifdef HAS_NETCDF
# include "netcdf.h"
#endif
#include "CheckRuns.h"
#include "Messages.h"
#include "TextFile.h"

#ifdef HAS_NETCDF
static inline int checkNCerr(int ncerr) {
  if ( ncerr != NC_NOERR ) {
    ErrorMsg("NETCDF: %s\n", nc_strerror(ncerr));
    return 1;
  }
  return 0;
}
#endif

static inline std::string Ext(std::string const& name) {
  size_t found = name.find_last_of(".");
  if (found == std::string::npos)
    return std::string("");
  return  name.substr(found);
}

/** Given two arrays of file names of different size try to determine
  * where they differ based on extension.
  */
static void CompareStrArray(StrArray const& a1, StrArray const& a2) {
  unsigned int max = (unsigned int)std::min(a1.size(), a2.size());
  unsigned int idx = 0;
  for (idx = 0; idx != max; idx++) {
    std::string e1 = Ext(a1[idx]);
    std::string e2 = Ext(a2[idx]);
    if (e1 != e2) {
      ErrorMsg("Differs at '%s', '%s'\n", a1[idx].c_str(), a2[idx].c_str());
      return;
    }
  }
  if (idx < a1.size())
    ErrorMsg("Differs at '%s'\n", a1[idx].c_str());
  if (idx < a2.size())
    ErrorMsg("Differs at '%s'\n", a2[idx].c_str());
}

int CheckRuns(std::string const& TopDir, StrArray const& RunDirs, bool firstOnly) {
#ifdef HAS_NETCDF
  if (firstOnly)
    Msg("Checking only first output/traj for all runs.\n");
  else
    Msg("Checking all output/traj for all runs.\n");
  int debug = 0;
  int Nwarnings = 0;
  for (StrArray::const_iterator rdir = RunDirs.begin(); rdir != RunDirs.end(); ++rdir) {
    if (ChangeDir( TopDir )) return 1;
    if (!fileExists( *rdir ))
      Msg("Warning: '%s' does not exist.\n", rdir->c_str());
    else {
      Msg("  %s:", rdir->c_str());
      ChangeDir( *rdir );
      bool is_md = false;
      // Determine where the output file(s) are.
      StrArray output_files = ExpandToFilenames("OUTPUT/rem.out.*");
      if (output_files.empty()) {
        output_files = ExpandToFilenames("md.out.*");
        is_md = true;
      }
      Msg(" %zu output files.\n", output_files.size());
      if (output_files.empty()) {
        ErrorMsg("Output files not found.\n");
        return 1;
      }
      // Determine where the trajectory files are.
      StrArray traj_files;
      if (!is_md)
        traj_files = ExpandToFilenames("TRAJ/rem.crd.*");
      else
        traj_files = ExpandToFilenames("md.nc.*");
      if (traj_files.size() != output_files.size()) {
        ErrorMsg("Number of output files %zu != # of traj files %zu.\n",
                 output_files.size(), traj_files.size());
        CompareStrArray( output_files, traj_files );
        return 1;
      }
      // Loop over output and trajectory files.
      int badFrameCount = -1;
      int numBadFrameCount = 0;
      bool check_restarts = false;
      StrArray::const_iterator tname = traj_files.begin();
      for (StrArray::const_iterator fname = output_files.begin();
                                    fname != output_files.end();
                                  ++fname, ++tname)
      {
        if (debug > 0) Msg("    '%s'\n", fname->c_str());
        // Determine how many frames should be written by the output file.
        TextFile mdout;
        if (mdout.OpenRead( *fname )) return 1;
        int readInput = 0;
        int nstlim = 0;
        double dt = 0;
        int numexchg = 0;
        int ntwx = 0;
        int expectedFrames = 0;
        const char* SEP = " ,=";
        int ncols = mdout.GetColumns(SEP);
        while (ncols > -1) {
          if (readInput == 0 && ncols > 2) {
            if (mdout.Token(0) == "2." && mdout.Token(1) == "CONTROL")
              readInput = 1;
          } else if (readInput == 1 && ncols > 1) {
            if (mdout.Token(0) == "3." && mdout.Token(1) == "ATOMIC")
              break;
            for (int col = 0; col != ncols - 1; col++) {
              if (mdout.Token(col) == "nstlim")
                nstlim = atoi( mdout.Token(col+1).c_str() );
              else if (mdout.Token(col) == "dt")
                dt = atof( mdout.Token(col+1).c_str() );
              else if (mdout.Token(col) == "numexchg")
                numexchg = atoi( mdout.Token(col+1).c_str() );
              else if (mdout.Token(col) == "ntwx")
                ntwx = atoi( mdout.Token(col+1).c_str() );
            }
          }
          ncols = mdout.GetColumns(SEP);
        }
        mdout.Close();
        //Msg("\tnstlim= %i\n", nstlim);
        //Msg("\tdt= %g\n", dt);
        //Msg("\tnumexchg= %i\n", numexchg);
        //Msg("\tntwx= %i\n", ntwx);
        if (numexchg == 0) numexchg = 1;
        double totalTime = ((double)nstlim * dt) * (double)numexchg;
        expectedFrames = (nstlim * numexchg) / ntwx;
        if (debug > 0) {
          Msg("\tTotal time: %g ps\n", totalTime);
          Msg("\tFrames: %i\n", expectedFrames);
        }
        // Get actual number of frames from NetCDF file.
        int ncid = -1;
        if ( checkNCerr(nc_open(tname->c_str(), NC_NOWRITE, &ncid)) ) return 1;
        int dimID;
        size_t slength = 0;
        if ( checkNCerr(nc_inq_dimid(ncid, "frame", &dimID))  ) return 1;
        if ( checkNCerr(nc_inq_dimlen(ncid, dimID, &slength)) ) return 1;
        int actualFrames = (int)slength;
        nc_close( ncid );
        if (debug > 0) Msg("\tActual Frames: %i\n", actualFrames);
        // If run did not complete, check restart files if replica.
        if (expectedFrames != actualFrames) {
          ++numBadFrameCount;
          ++Nwarnings;
          if (badFrameCount != actualFrames) { // To avoid repeated checkall warnings
            Msg("Warning: # actual frames %i != # expected frames %i.\n",
                actualFrames, expectedFrames);
            badFrameCount = actualFrames;
          }
          if (!is_md) check_restarts = true;
        } else {
          if (debug > 0) Msg("\tOK.\n");
        }
        if (firstOnly) break;
      } // END loop over output files for run
      if (numBadFrameCount > 0)
        Msg("Warning: Frame count did not match for %i replicas.\n", numBadFrameCount);
      if (check_restarts) {
        StrArray restart_files = ExpandToFilenames("RST/*.rst7");
        if (restart_files.empty())
          restart_files = ExpandToFilenames("RST/*.ncrst");
        if (restart_files.size() != output_files.size()) {
          ErrorMsg("Number of restart files %zu != # output files %zu\n",
                   restart_files.size(), output_files.size());
          CompareStrArray( restart_files, output_files );
          return 1;
        }
        double rst_time0 = 0.0;
        for (StrArray::const_iterator rfile = restart_files.begin();
                                      rfile != restart_files.end(); ++rfile)
        {
          int ncid = -1, timeVID = -1;
          double rsttime = -1.0;
          if ( checkNCerr(nc_open(rfile->c_str(), NC_NOWRITE, &ncid)) ) return 1; // TODO Ascii
          if ( checkNCerr(nc_inq_varid(ncid, "time", &timeVID)      ) ) return 1;
          if ( checkNCerr(nc_get_var_double(ncid, timeVID, &rsttime)) ) return 1;
          if (rfile == restart_files.begin()) {
            rst_time0 = rsttime;
            Msg("\tInitial restart time: %g\n", rst_time0);
          } else if ( fabs(rst_time0 - rsttime) > 0.00000000000001 ) {
            ErrorMsg("File %s time %g does not match initial restart time %g\n",
                     rfile->c_str(), rsttime, rst_time0);
            return 1;
          }
        }
      }
    }
  } // END loop over runs
  if (Nwarnings == 0)
    Msg("  All checks OK.\n");
  else
    Msg("  Runs seem OK, but some warnings were encountered.\n");
# else
  Msg("Warning: Compiled without NetCDF. Checking of runs is disabled.\n");
# endif
  return 0;
}
