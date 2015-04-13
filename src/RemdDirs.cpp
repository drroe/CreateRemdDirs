#include <cstdlib> // atoi, atof
#include <cstring> // strtok
#include "RemdDirs.h"
#include "Messages.h"
#include "FileRoutines.h"
#include "TextFile.h"

RemdDirs::RemdDirs() : nstlim_(-1), ig_(-1), numexchg_(-1),
   dt_(-1.0), temp0_(-1.0), totalReplicas_(0), top_dim_(-1),
   temp0_dim_(-1), debug_(0), n_md_runs_(0), umbrella_(0)
{}

// DESTRUCTOR
RemdDirs::~RemdDirs() {
  for (DimArray::const_iterator dim = Dims_.begin(); dim != Dims_.end(); ++dim)
    if (*dim != 0) delete *dim;
}

void RemdDirs::OptHelp() {
  Msg("Input file variables:\n"
      "  CRD_FILE <dir>         : Starting coordinates location.\n"
      "  DIMENSION <file>       : File containing replica dimension information, 1 per dimension\n"
//      "  START_RUN <start run>  : Run number to start with (run.<start_run>).\n"
//      "  STOP_RUN <stop run>    : Run number to finish with (run.<stop run>).\n"
      "  TRAJOUTARGS <args>     : Additional trajectory output args for analysis (--analyze).\n"
      "  FULLARCHIVE <arg>      : Comma-separated list of members to fully archive or NONE.\n"
      "  TOPOLOGY <file>        : Topology for 1D TREMD run.\n"
      "  MDIN_FILE <file>       : File containing extra MDIN input.\n"
      "  RST_FILE <file>        : File containing NMR restraints (MD only).\n"
      "  TEMPERATURE <T>        : Temperature for 1D HREMD run.\n"
      "  NSTLIM <nstlim>        : Input file; steps per exchange. Required.\n"
      "  DT <step>              : Input file; time step. Required.\n"
      "  IG <seed>              : Input file; random seed.\n"
      "  MDRUNS <#>             : Number of MD runs when not REMD (default 1).\n"
      "  NUMEXCHG <#>           : Input file; number of exchanges. Required for REMD.\n"
      "  UMBRELLA <#>           : Indicates MD umbrella sampling with write frequency <#>.\n\n");
}

// RemdDirs::ReadOptions()
int RemdDirs::ReadOptions(std::string const& input_file) {
  // Read options from input file
  if (CheckExists("Input file", input_file)) return 1;
  Msg("Reading input from file: %s\n", input_file.c_str());
  TextFile infile;
  if (infile.OpenRead(input_file)) return 1;
  int err = 0;
  const char* SEP = " \n";
  char* buffer;
  while ( (buffer = infile.Gets()) != 0) {
    Sarray tokens;
    char* ptr = strtok(buffer, SEP);
    while ( ptr != 0 ) {
      tokens.push_back( std::string(ptr) );
      ptr = strtok(0, SEP);
    }
    if (!tokens.empty()) {
      if (tokens.size() < 2) {
        ErrorMsg("Malformed input: %s\n", buffer);
        err = 1;
        OptHelp();
        break;
      }
      std::string OPT = tokens[0];
      // Skip comment lines
      if (OPT[0]=='#') continue;
      std::string VAR = tokens[1];
      for (unsigned int i = 2; i < tokens.size(); i++)
        VAR += (" " + tokens[i]);
      if (debug_ > 0)
        Msg("    Option: %s  Variable: %s\n", OPT.c_str(), VAR.c_str());
      if (OPT == "CRD_FILE")
        crd_dir_ = VAR;
      else if (OPT == "DIMENSION")
      {
        if (CheckExists("Dimension file", VAR)) { err = 1; break; }
        DimFileNames_.push_back( tildeExpansion(VAR) );
      }
      else if (OPT == "MDRUNS")
        n_md_runs_ = atoi( VAR.c_str() );
      else if (OPT == "NSTLIM")
        nstlim_ = atoi( VAR.c_str() );
      else if (OPT == "DT")
        dt_ = atof( VAR.c_str() );
      else if (OPT == "IG")
        ig_ = atoi( VAR.c_str() );
      else if (OPT == "NUMEXCHG")
        numexchg_ = atoi( VAR.c_str() );
      else if (OPT == "UMBRELLA")
        umbrella_ = atoi( VAR.c_str() );
      else if (OPT == "TOPOLOGY")
      {
        top_file_ = VAR;
        // If the TOPOLOGY exists at this point assume it is an absolute path
        // and perform tildeExpansion.
        if (fileExists(top_file_))
          top_file_ = tildeExpansion( top_file_ );
      }
      else if (OPT == "TEMPERATURE")
        temp0_ = atof( VAR.c_str() );
      else if (OPT == "TRAJOUTARGS")
        trajoutargs_ = VAR;
      else if (OPT == "FULLARCHIVE")
        fullarchive_ = VAR;
      else if (OPT == "MDIN_FILE")
      {
        if (CheckExists("MDIN file", VAR)) { err = 1; break; }
        mdin_file_ = tildeExpansion( VAR );
      }
      else if (OPT == "RST_FILE")
      {
        rst_file_ = VAR;
        if (fileExists(rst_file_))
          rst_file_ = tildeExpansion( rst_file_ );
      }
      else
      {
        ErrorMsg("Unrecognized option '%s' in input file.", OPT.c_str());
        err = 1;
        OptHelp();
        break;
      }
    }
  }
  infile.Close();
  // If MDIN file specified, store it in a string.
  additionalInput_.clear();
  if (!mdin_file_.empty()) {
    TextFile MDIN;
    if (MDIN.OpenRead(mdin_file_)) return 1;
    char* buffer;
    while ( (buffer = MDIN.Gets()) != 0)
      additionalInput_.append( buffer );
    MDIN.Close();
  }
  return err;
}

// RemdDirs::LoadDimensions()
int RemdDirs::LoadDimensions() {
  TextFile infile;
  int err = 0;
  totalReplicas_ = 1;
  for (Sarray::const_iterator dfile = DimFileNames_.begin();
                              dfile != DimFileNames_.end(); ++dfile)
  {
    // File existence already checked.
    if (infile.OpenRead(*dfile)) return 1;
    // Determine dimension type from first line.
    std::string firstLine = infile.GetString();
    if (firstLine.empty()) {
      ErrorMsg("Could not read first line of dimension file '%s'\n", dfile->c_str());
      err = 1;
      break;
    }
    infile.Close(); 
    // Allocate proper dimension type and load.
    ReplicaDimension* dim = ReplicaAllocator::Allocate( firstLine );
    if (dim == 0) {
      ErrorMsg("Unrecognized dimension type: %s\n", firstLine.c_str());
      err = 2;
      break;
    }
    // Push it here so it will be deallocated if there is an error
    Dims_.push_back( dim ); 
    if (dim->LoadDim( *dfile )) {
      ErrorMsg("Loading info from dimension file '%s'\n", dfile->c_str());
      err = 3;
      break;
    }
    Msg("\t%u: %s (%u)\n", Dims_.size(), dim->description(), dim->Size());
    totalReplicas_ *= dim->Size();
  }
  if (err != 0) return err;
  Msg("    %u total replicas.\n", totalReplicas_);
  if (Dims_.empty()) {
    ErrorMsg("No dimensions defined.\n");
    return 1;
  }
  // Do some error checking.
  temp0_dim_ = -1;
  top_dim_ = -1;
  int providesTemp0 = 0;
  int providesTopFiles = 0;
  for (DimArray::const_iterator dim = Dims_.begin(); dim != Dims_.end(); ++dim)
  {
    if ((*dim)->ProvidesTemp0()) {
      temp0_dim_ = (int)(dim - Dims_.begin());
      providesTemp0++;
    }
    if ((*dim)->ProvidesTopFiles()) {
      top_dim_ = (int)(dim - Dims_.begin());
      providesTopFiles++;
    }
  }
  if (providesTemp0 > 1) {
    ErrorMsg("At most one dimension that provides temperatures should be specified.\n");
    return 1;
  } else if (providesTemp0 == 0 && temp0_ < 0.0) {
    ErrorMsg("No dimension provides temperature and TEMPERATURE not specified.\n");
    return 1;
  }
  if (providesTopFiles > 1) {
    ErrorMsg("At most one dimension that provides topology files should be specified.\n");
    return 1;
  } else if (providesTopFiles == 0 && top_file_.empty()) {
    ErrorMsg("No dimension provides topology files and TOPOLOGY not specified.\n");
    return 1;
  }
  // Determine run type
  run_type_.clear();
  if (Dims_.size() == 1)
    run_type_.assign( Dims_[0]->name() );
  else // Must be > 1
    run_type_ = "MREMD";
  Msg("Run type: %s\n", run_type_.c_str());
  if (debug_ > 0)
    Msg("    Topology dimension: %i\n    Temp0 dimension: %i\n", top_dim_, temp0_dim_);
  return 0;
}

// =============================================================================
// RemdDirs::CreateRun()
int RemdDirs::CreateRun(int start_run, int run_num, std::string const& run_dir) {
  typedef std::vector<unsigned int> Iarray;
  // Create and change to run directory.
  if (Mkdir(run_dir)) return 1;
  if (ChangeDir(run_dir)) return 1;
  // Ensure that coords directory exists.
  if (!fileExists(crd_dir_)) {
    ErrorMsg("Coords directory '%s' not found. Must specify absolute path"
             " or path relative to '%s'\n", crd_dir_.c_str(), run_dir.c_str());
    return 1;
  }
  // Calculate ps per exchange
  double ps_per_exchg = dt_ * (double)nstlim_;
  // Do we need to setup groups for MREMD?
  bool setupGroups = (groups_.Empty() && Dims_.size() > 1);
  if (setupGroups)
    groups_.SetupGroups( Dims_.size() );
  // Create INPUT directory if not present.
  std::string input_dir("INPUT");
  if (Mkdir(input_dir)) return 1;
  // Open GROUPFILE
  TextFile GROUPFILE;
  if (GROUPFILE.OpenWrite("groupfile")) return 1; 
  // Figure out max width of replica extension
  int width = std::max(DigitWidth( totalReplicas_ ), 3);
  // Hold current indices in each dimension.
  Iarray Indices( Dims_.size(), 0 );
  std::string currentTop;
  double currentTemp0;
  if (top_dim_ == -1) currentTop = top_file_;
  if (temp0_dim_ == -1) currentTemp0 = temp0_;
  for (unsigned int rep = 0; rep != totalReplicas_; rep++)
  {
    // Get topology/temperature for this replica if necessary.
    if (top_dim_ != -1) currentTop = Dims_[top_dim_]->TopName( Indices[top_dim_]  );
    if (temp0_dim_ != -1) currentTemp0 = Dims_[temp0_dim_]->Temp0( Indices[temp0_dim_] );
    // Ensure topology exists.
    if (!fileExists( currentTop )) {
      ErrorMsg("Topology '%s' not found. Must specify absolute path"
               " or path relative to '%s'\n", currentTop.c_str(), run_dir.c_str());
      return 1;
    }
    // Info for this replica.
    if (debug_ > 1) {
      Msg("\tReplica %u: top=%s  temp0=%f", rep+1, currentTop.c_str(), currentTemp0);
      Msg("  {");
      for (Iarray::const_iterator count = Indices.begin(); count != Indices.end(); ++count)
        Msg(" %u", *count);
      Msg(" }\n");
    }
    // Save group info
    if (setupGroups)
      groups_.AddReplica( Indices, rep+1 );
    // Replica extension. 
    std::string EXT = integerToString(rep+1, width);
    // Create input
    int irest = 1;
    int ntx = 5;
    if (run_num == 0) {
      if (rep ==0)
        Msg("    Run 0: irest=0, ntx=1\n");
      irest = 0;
      ntx = 1;
    }
    std::string mdin_name(input_dir + "/in." + EXT);
    if (debug_ > 1)
      Msg("\t\tMDIN: %s\n", mdin_name.c_str());
    TextFile MDIN;
    if (MDIN.OpenWrite(mdin_name)) return 1;
    MDIN.Printf("%s", run_type_.c_str());
    // Write indices to mdin for MREMD
    if (Dims_.size() > 1) {
      MDIN.Printf(" {");
      for (Iarray::const_iterator count = Indices.begin(); count != Indices.end(); ++count)
        MDIN.Printf(" %u", *count + 1);
      MDIN.Printf(" }");
    }
    // for Top %u at %g K 
    MDIN.Printf(" (rep %u), %g ps/exchg\n"
                " &cntrl\n"
                "    imin = 0, nstlim = %i, dt = %f,\n"
                "    irest = %i, ntx = %i, ig = %i, numexchg = %i,\n"
                "    temp0 = %f, tempi = %f,\n%s",
                rep+1, ps_per_exchg,
                nstlim_, dt_, irest, ntx, ig_, numexchg_,
                currentTemp0, currentTemp0, additionalInput_.c_str());
    for (unsigned int id = 0; id != Dims_.size(); id++)
      Dims_[id]->WriteMdin(Indices[id], MDIN);
    MDIN.Printf(" &end\n");
    MDIN.Close();
    // Write to groupfile
    std::string INPUT_CRD = crd_dir_ + "/" + EXT + ".rst7";
    if (run_num == start_run && !fileExists( INPUT_CRD )) {
      ErrorMsg("Coords %s not found.", INPUT_CRD.c_str());
      return 1;
    }
    if (debug_ > 1)
      Msg("\t\tINPCRD: %s\n", INPUT_CRD.c_str());
    std::string GROUPFILE_LINE = "-O -remlog rem.log -i " + mdin_name +
      " -p " + currentTop + " -c " + INPUT_CRD + " -o OUTPUT/rem.out." + EXT +
      " -inf INFO/reminfo." + EXT + " -r RST/" + EXT + 
      ".rst7 -x TRAJ/rem.crd." + EXT + " -l LOG/logfile." + EXT;
    for (unsigned int id = 0; id != Dims_.size(); id++)
      GROUPFILE_LINE += Dims_[id]->Groupline(EXT);
    GROUPFILE.Printf("%s\n", GROUPFILE_LINE.c_str());
    // Increment first (fastest growing) index.
    Indices[0]++;
    // Increment remaining indices if necessary.
    for (unsigned int id = 0; id != Dims_.size() - 1; id++)
    {
      if (Indices[id] == Dims_[id]->Size()) {
        Indices[id] = 0; // Set this index to zero.
        Indices[id+1]++; // Increment next index.
      }
    }
  }
  GROUPFILE.Close();
  if (debug_ > 1 && !groups_.Empty())
    groups_.PrintGroups();
  // Create remd.dim if necessary.
  if (Dims_.size() > 1) {
    TextFile REMDDIM;
    if (REMDDIM.OpenWrite("remd.dim")) return 1;
    for (unsigned int id = 0; id != Dims_.size(); id++)
      groups_.WriteRemdDim(REMDDIM, id, Dims_[id]->exch_type(), Dims_[id]->description());
    REMDDIM.Close();
  }
  // Create output directories
  if (Mkdir( "OUTPUT" )) return 1;
  if (Mkdir( "TRAJ"   )) return 1;
  if (Mkdir( "RST"    )) return 1;
  if (Mkdir( "INFO"   )) return 1;
  if (Mkdir( "LOG"    )) return 1;
  // Create any dimension-specific directories
  for (DimArray::const_iterator dim = Dims_.begin(); dim != Dims_.end(); ++dim) {
    if ((*dim)->OutputDir() != 0) {
      if (Mkdir( std::string((*dim)->OutputDir()))) return 1;
    }
  }
  // Input coordinates for next run will be restarts of this
  crd_dir_ = "../" + run_dir + "/RST";
  return 0;
}

// =============================================================================
/** Create input file for MD.
  * \param fname Name of MDIN file.
  * \param run_num Run number, for setting irest/ntx.
  * \param EXT Extension for restraint/dumpave files when umbrella sampling.
  */
int RemdDirs::MakeMdinForMD(std::string const& fname, int run_num, 
                            std::string const& EXT, std::string const& run_dir) const
{
  // Create input
  double total_time = dt_ * (double)nstlim_;
  int irest = 1;
  int ntx = 5;
  if (run_num == 0) {
    Msg("    Run 0: irest=0, ntx=1\n");
    irest = 0;
    ntx = 1;
  }
  TextFile MDIN;
  if (MDIN.OpenWrite(fname)) return 1;
  MDIN.Printf("%s %g ps\n"
              " &cntrl\n"
              "    imin = 0, nstlim = %i, dt = %f,\n"
              "    irest = %i, ntx = %i, ig = %i,\n"
              "    temp0 = %f, tempi = %f,\n%s",
              run_type_.c_str(), total_time,
              nstlim_, dt_, irest, ntx, ig_,
              temp0_, temp0_, additionalInput_.c_str());
  if (!rst_file_.empty()) {
    MDIN.Printf("    nmropt=1,\n");
    Msg("    Using NMR restraints.\n");
  }
  MDIN.Printf(" &end\n");
  if (!rst_file_.empty()) {
    // Restraints
    std::string rf_name(rst_file_ + EXT);
    // Ensure restraint file exists if specified.
    if (!fileExists( rf_name )) {
      ErrorMsg("Restraint file '%s' not found. Must specify absolute path"
               " or path relative to '%s'\n", rf_name.c_str(), run_dir.c_str());
      return 1;
    }
    if (umbrella_ > 0)
      MDIN.Printf("&wt\n   TYPE=\"DUMPFREQ\", istep1 = %i,\n&end\n", umbrella_);
    MDIN.Printf("&wt\n   TYPE=\"END\",\n&end\nDISANG=%s\n", rf_name.c_str());
    if (umbrella_ > 0) // TODO: customize dumpave name?
      MDIN.Printf("DUMPAVE=dumpave%s\n", EXT.c_str());
    MDIN.Printf("/\n");
  }
  MDIN.Close();
  return 0;
}

// RemdDirs::CreateMD()
int RemdDirs::CreateMD(int start_run, int run_num, std::string const& run_dir) {
  run_type_.assign("MD");
  // Create and change to run directory.
  if (Mkdir(run_dir)) return 1;
  if (ChangeDir(run_dir)) return 1;
  // Do some set up for groupfile runs.
  int width = 3;
  std::vector<std::string> crd_files;
  if (n_md_runs_ > 1) {
    Msg("Number of MD runs: %i\n", n_md_runs_);
    // Figure out max width of md filename extension
    width = std::max(DigitWidth( n_md_runs_ ), 3);
    for (int grp=1; grp <= n_md_runs_; grp++)
      crd_files.push_back(crd_dir_ + "/" + integerToString(grp, width) + ".rst7");
  }
  // Ensure that coords file exists for first run.
  if (run_num == start_run) {
    if (n_md_runs_ < 2) {
      if (!fileExists(crd_dir_)) {
        ErrorMsg("Coords file '%s' not found. Must specify absolute path"
                 " or path relative to '%s'\n", crd_dir_.c_str(), run_dir.c_str());
        return 1;
      }
    } else {
      for (std::vector<std::string>::const_iterator file = crd_files.begin();
                                                    file != crd_files.end(); ++file)
        if (!fileExists(*file)) {
          ErrorMsg("Coords file '%s' not found. Must specify absolute path"
                 " or path relative to '%s'\n", file->c_str(), run_dir.c_str());
          return 1;
        }
    }
  }
  // Ensure topology exists.
  if (!fileExists( top_file_ )) {
    ErrorMsg("Topology '%s' not found. Must specify absolute path"
             " or path relative to '%s'\n", top_file_.c_str(), run_dir.c_str());
    return 1;
  }
  // Groupfile will be used by MasterQsub.sh for command-line flags.
  TextFile GROUP;
  if (GROUP.OpenWrite("groupfile")) return 1;
  if (n_md_runs_ < 2)
    GROUP.Printf("-i md.in -p %s -c %s -x mdcrd.nc -r mdrst.rst7 -o md.out -inf md.info\n", 
                 top_file_.c_str(), crd_dir_.c_str());
  else {
    for (int grp = 1; grp <= n_md_runs_; grp++) {
      std::string EXT = "." + integerToString(grp, width);
      std::string mdin_name("md.in");
      if (umbrella_ > 0) {
        // Create input for umbrella runs
        mdin_name.append(EXT);
        if (MakeMdinForMD(mdin_name, run_num, EXT, run_dir)) return 1;
      }
      GROUP.Printf("-i %s -p %s -c %s -x md.nc%s -r %0*i.rst7 -o md.out%s -inf md.info%s\n",
                   mdin_name.c_str(), top_file_.c_str(), crd_files[grp-1].c_str(), EXT.c_str(),
                   width, grp, EXT.c_str(), EXT.c_str());
    } 
  }
  GROUP.Close();
  // Info for this run.
  if (debug_ >= 0) // 1 
      Msg("\tMD: top=%s  temp0=%f\n", top_file_.c_str(), temp0_);
  // Create input for non-umbrella runs.
  if (umbrella_ == 0) {
    if (MakeMdinForMD("md.in", run_num, "",run_dir)) return 1;
  }
  // Input coordinates for next run will be restarts of this
  crd_dir_ = "../" + run_dir + "/";
  if (n_md_runs_ < 2) crd_dir_.append("mdrst.rst7");
  return 0;
}
