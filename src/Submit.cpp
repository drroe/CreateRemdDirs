#include <cstdlib> // atoi
#include "Submit.h"
#include "Messages.h"
#include "TextFile.h"
#include "FileRoutines.h"


Submit::~Submit() {
  if (Run_ != 0) delete Run_;
  if (Analyze_ != 0) delete Analyze_;
  if (Archive_ != 0) delete Archive_;
}

int Submit::WriteRuns(std::string const& top) const { return Run_->Write(top); }

int Submit::ReadOptions(std::string const& fn) {
  if (Run_ != 0) {
    ErrorMsg("Only one queue input allowed.\n");
    return 1;
  }
  Run_ = new QueueOpts();
  if (ReadOptions( fn, *Run_ )) return 1;
  if (Run_->Check()) return 1;
  if (Analyze_ != 0 && Analyze_->Check()) return 1;
  if (Archive_ != 0 && Archive_->Check()) return 1;
  return 0;
}

int Submit::ReadOptions(std::string const& fnameIn, QueueOpts& Qopt) {
  n_input_read_++;
  if (n_input_read_ > 100) {
    ErrorMsg("# of input files read > 100; possible infinite recursion.\n");
    return 1;
  }

  Msg("  Reading queue options from '%s'\n", fnameIn.c_str());
  std::string fname = tildeExpansion( fnameIn );
  if (CheckExists( "Queue options", fname )) return 1;
  TextFile infile;
  if (infile.OpenRead( fname )) return 1;
  const char* ptr = infile.Gets();
  while (ptr != 0) {
    // Format is <NAME> <OPTIONS>
    std::string line( ptr );
    size_t found = line.find_first_of(" ");
    if (found == std::string::npos) {
      ErrorMsg("malformed option: %s\n", ptr);
      return 1;
    }
    size_t found1 = found;
    while (found1 < line.size() && line[found1] == ' ') ++found1;
    std::string Args = line.substr(found1);
    // Remove any newline chars
    found1 = Args.find_first_of("\r\n");
    if (found1 != std::string::npos) Args.resize(found1);
    // Reduce line to option
    line.resize(found);
    Msg("Opt: '%s'   Args: '%s'\n", line.c_str(), Args.c_str());

    // Process options
    if (line == "ANALYZE_FILE") {
      if (Analyze_ != 0) {
        ErrorMsg("Only one ANALYZE_FILE allowed.\n");
        return 1;
      }
      Analyze_ = new QueueOpts(); // TODO Copy of existing?
      if (ReadOptions( Args, *Analyze_ )) return 1;
    } else if (line == "ARCHIVE_FILE") {
      if (Archive_ != 0) {
        ErrorMsg("Only one ARCHIVE_FILE allowed.\n");
        return 1;
      }
      Archive_ = new QueueOpts();
      if (ReadOptions( Args, *Archive_ )) return 1;
    } else if (line == "INPUT_FILE") {
      // Try to prevent recursion.
      std::string fn = tildeExpansion( Args );
      if (fn == fname) {
        ErrorMsg("An input file may not read from itself (%s).\n", Args.c_str());
        return 1;
      }
      if (ReadOptions( Args, Qopt )) return 1;
    } else {
      if (Qopt.ProcessOption(line, Args)) return 1;
    }
    
    ptr = infile.Gets();
  }
  infile.Close();

  return 0;
}
  

// =============================================================================
Submit::QueueOpts::QueueOpts() :
  nodes_(0),
  ng_(0),
  ppn_(0),
  threads_(0),
  runType_(MD),
  overWrite_(false),
  testing_(false),
  queueType_(PBS),
  isSerial_(false),
  dependType_(BATCH),
  setupDepend_(true)
{}

const char* Submit::QueueOpts::RunTypeStr[] = {
  "MD", "TREMD", "HREMD", "MREMD", "ANALYSIS", "ARCHIVE"
};

const char* Submit::QueueOpts::QueueTypeStr[] = {
  "PBS", "SBATCH"
};

static inline int RetrieveOpt(const char** Str, int end, std::string const& VAR) {
  for (int i = 0; i != end; i++)
    if ( VAR.compare( Str[i] )==0 )
      return i;
  return end;
}

int Submit::QueueOpts::ProcessOption(std::string const& OPT, std::string const& VAR) {
  Msg("Processing '%s' '%s'\n", OPT.c_str(), VAR.c_str()); // DEBUG

  if      (OPT == "JOBNAME") job_name_ = VAR;
  else if (OPT == "NODES"  ) nodes_ = atoi( VAR.c_str() );
  else if (OPT == "NG"     ) ng_ = atoi( VAR.c_str() );
  else if (OPT == "PPN"    ) ppn_ = atoi( VAR.c_str() );
  else if (OPT == "THREADS") threads_ = atoi( VAR.c_str() );
  else if (OPT == "RUNTYPE") {
    runType_ = (RunType)RetrieveOpt(RunTypeStr, NO_RUN, VAR);
    if (runType_ == NO_RUN) {
      ErrorMsg("Unrecognized run type: %s\n", VAR.c_str());
      return 1;
    }
  }
  else if (OPT == "AMBERHOME") {
    if ( CheckExists("AMBERHOME", VAR) ) return 1;
    amberhome_ = tildeExpansion( VAR );
  }
  else if (OPT == "PROGRAM"  ) program_ = VAR;
  else if (OPT == "QSUB"     ) {
    queueType_ = (QueueType) RetrieveOpt(QueueTypeStr, NO_QUEUE, VAR);
    if (queueType_ == NO_QUEUE) {
      ErrorMsg("Unrecognized QSUB: %s\n", VAR.c_str());
      return 1;
    }
  }
  else if (OPT == "WALLTIME"  ) walltime_ = VAR;
  else if (OPT == "NODEARGS"  ) nodeargs_ = VAR;
  else if (OPT == "MPIRUN"    ) mpirun_ = VAR;
  else if (OPT == "MODULEFILE") {
    if ( CheckExists( "Module file", VAR ) ) return 1;
    TextFile modfile;
    if (modfile.OpenRead( tildeExpansion(VAR) )) return 1;
    const char* ptr = modfile.Gets();
    while (ptr != 0) {
      additionalCommands_.append( std::string(ptr) );
      ptr = modfile.Gets();
    }
    modfile.Close();
  }
  else if (OPT == "ACCOUNT") account_ = VAR;
  else if (OPT == "EMAIL"  ) email_ = VAR;
  else if (OPT == "QUEUE"  ) queueName_ = VAR;
  else if (OPT == "SERIAL" ) isSerial_ = (bool)atoi( VAR.c_str() );
  else if (OPT == "CHAIN"  ) {
    // TODO modify if more chain options introduced
    int ival = atoi( VAR.c_str() );
    if (ival == 1) dependType_ = SUBMIT;
  }
  else if (OPT == "NO_DEPEND") setupDepend_ = !((bool)atoi( VAR.c_str() ));
  else if (OPT == "FLAG"     ) Flags_.push_back( VAR );
  else {
    ErrorMsg("Unrecognized option '%s' in input file.\n", OPT.c_str());
    return 1;
  }
  return 0;
}

int Submit::QueueOpts::Check() const {
  if (job_name_.empty()) {
    ErrorMsg("No job name\n");
    return 1;
  }
  if (program_.empty()) { // TODO check AMBERHOME ?
    ErrorMsg("PROGRAM not specified.\n");
    return 1;
  }
  if (!isSerial_ && mpirun_.empty()) {
    ErrorMsg("MPI run command MPIRUN not set.\n");
    return 1;
  }
  return 0;
}

int Submit::QueueOpts::Write(std::string const& WorkDir) const {
  // Get user name from whoami command
  std::string user = UserName();
  Msg("User: %s\n", user.c_str());
  return 0;
}
  
