#include <cmath>
#include <cstdlib>
#include "netcdf.h"
#include "CheckRuns.h"
#include "Messages.h"
#include "TextFile.h"

int CheckRuns(std::string const& TopDir, StrArray const& RunDirs) {
  Msg("Checking runs.\n");
  int debug = 0;
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
      // Determine where the trajectory files are.
      StrArray traj_files;
      if (!is_md)
        traj_files = ExpandToFilenames("TRAJ/rem.crd.*");
      else
        traj_files = ExpandToFilenames("md.nc.*");
      if (traj_files.size() != output_files.size()) {
        ErrorMsg("Number of output files %zu != # of traj files %zu.\n",
                 output_files.size(), traj_files.size());
        return 1;
      }
      // Loop over output and trajectory files.
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
        if ( nc_open(tname->c_str(), NC_NOWRITE, &ncid) != NC_NOERR ) return 1;
        int dimID;
        size_t slength = 0;
        if ( nc_inq_dimid(ncid, "frame", &dimID) != NC_NOERR ) return 1;
        if ( nc_inq_dimlen(ncid, dimID, &slength) != NC_NOERR ) return 1;
        int actualFrames = (int)slength;
        nc_close( ncid );
        if (debug > 0) Msg("\tActual Frames: %i\n", actualFrames);
        // If run did not complete, check restart files if replica.
        if (expectedFrames != actualFrames) {
          Msg("Warning: # actual frames %i != # expected frames.\n",
              actualFrames, expectedFrames);
          if (!is_md) check_restarts = true; 
        } else {
          if (debug > 0) Msg("\tOK.\n");
        }
      } // END loop over output files for run
      if (check_restarts) {
        StrArray restart_files = ExpandToFilenames("RST/*.rst7");
        if (restart_files.empty())
          restart_files = ExpandToFilenames("RST/*.ncrst");
        if (restart_files.size() != output_files.size()) {
          ErrorMsg("Number of restart files %zu != # output files %zu\n",
                   restart_files.size(), output_files.size());
          return 1;
        }
        double rst_time0 = 0.0;
        for (StrArray::const_iterator rfile = restart_files.begin();
                                      rfile != restart_files.end(); ++rfile)
        {
          int ncid = -1, timeVID = -1;
          double rsttime = -1.0;
          if ( nc_open(rfile->c_str(), NC_NOWRITE, &ncid) != NC_NOERR ) return 1; // TODO Ascii
          if ( nc_inq_varid(ncid, "time", &timeVID) != NC_NOERR ) return 1;
          if ( nc_get_var_double(ncid, timeVID, &rsttime) != NC_NOERR ) return 1;
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
  Msg("\tRun OK.\n");
  return 0;
}
