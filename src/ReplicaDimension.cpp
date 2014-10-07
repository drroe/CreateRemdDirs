#include <cstdio>
#include <sstream>   // istringstream, ostringstream
#include <cmath> // log10
#include "ReplicaDimension.h"
#include "FileRoutines.h"
#include "Messages.h"

// DigitWidth()
/** \return the number of characters necessary to express the given digit. */
int DigitWidth(long int numberIn) {
  if (numberIn == 0L) return 1;
  double numf = (double) numberIn;
  numf = log10( numf );
  ++numf;
  // The cast back to long int implicitly rounds down
  int numi = (int)numf;
  return numi;
}

// integerToString()
std::string integerToString(int i) {
  std::ostringstream oss;
  oss << i;
  return oss.str();
}

// integerToString()
std::string integerToString(int i, int width) {
  std::ostringstream oss;
  oss.fill('0');
  oss.width( width );
  oss << std::right << i;
  return oss.str();
}

// -----------------------------------------------------------------------------
const std::string ReplicaDimension::emptystring_ = "";

// Should correspong to ExchType
const char* ReplicaDimension::exchString_[] = { "NONE", "TEMPERATURE", "HAMILTONIAN" };

int TemperatureDim::LoadDim(std::string const& fname) {
  TextFile infile;
  if (infile.OpenRead(fname)) return 1;
  char* buffer = infile.Gets(); // Scan past first line.
  double temp0;
  while ( (buffer = infile.Gets()) != 0 ) {
    if (sscanf(buffer, "%lf", &temp0) != 1) {
      ErrorMsg("Reading temperature from dim file.\n");
      return 1;
    }
    temps_.push_back( temp0 );
  }
  infile.Close();
  std::ostringstream oss;
  oss << "Temperature exchange from " << temps_.front() << " K to " << temps_.back() << " K";
  SetDescription(oss.str());
  return 0;
}

// -----------------------------------------------------------------------------
int TopologyDim::LoadDim(std::string const& fname) {
  TextFile infile;
  if (infile.OpenRead(fname)) return 1;
  std::string topname = infile.GetString(); // Scan past first line.
  topname = infile.GetString(); // Get second line. 
  while ( !topname.empty() ) {
    if (CheckExists("Topology from dim", topname)) return 1;
    tops_.push_back( tildeExpansion(topname) );
    topname = infile.GetString();
  }
  infile.Close();
  SetDescription("Varying topology files");
  return 0;
}

// -----------------------------------------------------------------------------
/** Allow <top> (ignored) <alpha> <thresh> or <alpha> <thresh> */
int AmdDihedralDim::LoadDim(std::string const& fname) {
  TextFile infile;
  if (infile.OpenRead(fname)) return 1;
  char* buffer = infile.Gets(); // Scan past first line.
  int ncols = -1;
  char topname[1024];
  double alpha, thresh;
  int err = 0;
  while ( (buffer = infile.Gets()) != 0 ) {
    // Check which type this is
    if (ncols == -1) {
      ncols = sscanf(buffer, "%s %lf %lf", topname, &alpha, &thresh);
      if (ncols == 3)
        Msg("Warning: topologies from %s will be ignored.\n", fname.c_str());
    }
    // 2 cols - alpha, thresh
    if (ncols == 2) {
      if (sscanf(buffer, "%lf %lf", &alpha, &thresh) != 2) {err = 1; break;}
    } else if (ncols == 3) {
      if (sscanf(buffer, "%*s %lf %lf", &alpha, &thresh) != 2) { err = 1; break;}
    }
    d_alpha_.push_back( alpha );
    d_thresh_.push_back( thresh ); 
  }
  infile.Close();
  if (err != 0) {
    ErrorMsg("Reading alpha/threshold from dim file.\n");
    return 1;
  }
  SetDescription("AMD with various dihedral boost levels");
  return 0;
}

int AmdDihedralDim::WriteMdin(int idx, TextFile& mdin) const {
  if (d_thresh_[idx] > 0.0 || d_alpha_[idx] > 0.0)
    return mdin.Printf("    iamd=2, EthreshD=%f, alphaD=%f,\n", d_thresh_[idx], d_alpha_[idx]);
  return 0;
}

std::string AmdDihedralDim::Groupline(std::string const& EXT) const {
  return std::string(" -amd AMD/amd."+EXT);
}

// -----------------------------------------------------------------------------
int SgldDim::LoadDim(std::string const& fname) {
  TextFile infile;
  if (infile.OpenRead(fname)) return 1;
  char* buffer = infile.Gets(); // Scan past first line.
  double tempsg;
  while ( (buffer = infile.Gets()) != 0 ) {
    if (sscanf(buffer, "%lf", &tempsg) != 1) {
      ErrorMsg("Reading SGLD temperature from dim file.\n");
      return 1;
    }
    sgtemps_.push_back( tempsg );
  }
  infile.Close();
  std::ostringstream oss;
  oss << "RXSGLD from " << sgtemps_.front() << " K to " << sgtemps_.back() << " K"; 
  SetDescription(oss.str());
  return 0;
}

int SgldDim::WriteMdin(int idx, TextFile& mdin) const {
  return mdin.Printf("    isgld=1, tsgavg=0.2, tempsg=%f\n", sgtemps_[idx]);
}
