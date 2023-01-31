#include <cstdlib> // atoi
#include "Submitter.h"
#include "Messages.h"
#include "QueueOpts.h"
#include "StringRoutines.h"
#include "TextFile.h"
#include "OptArray.h"

using namespace Messages;
using namespace FileRoutines;
using namespace StringRoutines;

Submitter::~Submitter() {
  if (Run_ != 0) delete Run_;
  if (Analyze_ != 0) delete Analyze_;
  if (Archive_ != 0) delete Archive_;
}

void Submitter::OptHelp() {
  Msg("Queue job submission input file variables:\n"
      "  INPUT_FILE <file>  : Read additional options from <file>.\n"
      "  JOBNAME <name>     : Job name in queueing system (required).\n"
      "  NODES <#>          : Number of nodes needed.\n"
      "  PPN <#>            : Processors per node needed.\n"
      "  THREADS <#>        : Number of threads needed. Calcd from NODES * PPN if not specified\n"
      "  AMBERHOME <dir>    : Directory containing AMBER installation.\n"
      "  PROGRAM <name>     : Name of binary to run (required).\n"
      "  QSUB <arg>         : Queue type {PBS | SBATCH (slurm)}\n"
      "  WALLTIME <arg>     : Wall time needed.\n"
      "  NODEARGS <arg>     : Any additonal -l node arguments (PBS only)\n"
      "  MPIRUN <command>   : Command used to execute parallel run. Can use\n"
      "                       $NODES, $THREADS, $PPN (will be set by script).\n"
      "  MODULEFILE <file>  : File containing extra commands to run (only last one loaded used)\n"
      "  COMMANDFILE <file> : File containing any additional commands to be run.\n"
      "  COMMAND <command>  : Additional command to run (can specify multiple).\n"
      "  ACCOUNT <name>     : Account name\n"
      "  EMAIL <email>      : User email address\n"
      "  QUEUE <name>       : Queue name\n"
      "  SERIAL {0|1}       : If set to 1 run in serial, no MPIRUN needed.\n"
      "  DEPEND <arg>       : Job dependencies. BATCH=Use batch system (default),\n"
      "                       SUBMIT=Execute next script at end of previous, or NONE.\n"
      "  FLAG <flag>        : Any additional queue flags.\n\n");
}

int Submitter::SubmitRuns(std::string const& TopDir, StrArray const& RunDirs, int start, bool overwrite,
                       std::string const& prev_jobidIn)
const
{
  Run_->Info();
  std::string user = NoTrailingWhitespace( UserName() );
  Msg("User: %s\n", user.c_str());
  std::string jobIdFilename(TopDir + "/temp.jobid");
  std::string submitScript( std::string(Run_->SubmitCmd()) + ".sh" );
  std::string submitCommand( std::string(Run_->SubmitCmd()) + " " +
                             submitScript + " > " + jobIdFilename);
  std::string runScriptName("RunMD.sh");
  // Create run script for each run directory
  if (!prev_jobidIn.empty())
    Msg("First submission will depend on job id %s\n", prev_jobidIn.c_str());
  std::string previous_jobid = prev_jobidIn;
  int run_num;
  if (start != -1)
    run_num = start;
  else
    run_num = 0;
  Msg("Submitting %zu runs.\n", RunDirs.size());
  StrArray::const_iterator finaldir = RunDirs.end() - 1;
  for (StrArray::const_iterator rdir = RunDirs.begin(); rdir != RunDirs.end(); ++rdir)
  {
    ChangeDir( TopDir );
    // Check if run directories already contain scripts
    if ( !overwrite && fileExists( *rdir + "/" + submitScript) ) {
      ErrorMsg("Not overwriting (-O) and %s already contains %s\n",
               rdir->c_str(), submitScript.c_str());
      if (Run_->DependType() != QueueOpts::NONE) // Exit if dependencies exist
        return 1;
      else
        continue;
    }
    Msg("  %s\n", rdir->c_str());
    ChangeDir( *rdir );
    // Ensure runscript exists.
    if (CheckExists("run script", runScriptName)) return 1;
    // Set options specific to queuing system, node info, and Amber env.
    TextFile qout;
    if (qout.OpenWrite( submitScript )) return 1;
    if (Run_->QsubHeader(qout, run_num, previous_jobid, "")) return 1;
    // Set up command to execute run script
    qout.Printf("\n# Run executable\n./%s\n\n", runScriptName.c_str());
    // Set up script dependency if necessary
    if (Run_->DependType() == QueueOpts::SUBMIT && rdir != finaldir) {
      std::string next_dir("../" + *(rdir+1));
      qout.Printf("cd %s && %s %s\n", next_dir.c_str(), Run_->SubmitCmd(), submitScript.c_str());
    }
    qout.Printf("exit 0\n");
    qout.Close();
    ChangePermissions( submitScript );
    // Peform job submission if not testing
    if (testing_)
      Msg("Just testing. Skipping script submission.\n");
    else if (Run_->DependType() == QueueOpts::SUBMIT && rdir != RunDirs.begin())
      Msg("Job will be submitted when previous job completes.\n");
    else {
      Msg("%s\n", submitCommand.c_str()); 
      if ( system( submitCommand.c_str() ) ) {
        ErrorMsg("Job submission failed.\n");
        return 1;
      }
      // Get job ID of submitted job
      TextFile jobid;
      if (Run_->QueueType() == QueueOpts::PBS) {
        if (jobid.OpenRead( jobIdFilename )) return 1;
        const char* ptr = jobid.Gets();
        if (ptr == 0) return 1;
        previous_jobid.assign(ptr);
      } else if (Run_->QueueType() == QueueOpts::SLURM) {
        // -i inidcates reverse sort
        if (jobid.OpenPipe("squeue -u " + user + " --sort=-i")) return 1;
        const char* ptr = jobid.Gets();     // Header with JOBID
        if (ptr == 0) return 1;
        int cols = jobid.GetColumns(" \t"); // Should be last submitted job
        if (cols < 1) return 1;
        previous_jobid.assign( jobid.Token(0) );
      } else return 1; // sanity check
      jobid.Close();
      Msg("  Submitted: %s\n", previous_jobid.c_str());
      if (previous_jobid.empty() || previous_jobid == "JOBID") {
        ErrorMsg("Job not submitted.\n");
        return 1;
      }
      if (Run_->DependType() != QueueOpts::BATCH) previous_jobid.clear();
    }
    ++run_num;
  }

  return 0; 
}

int Submitter::SubmitAnalysis(std::string const& TopDir, int start, int stop, bool overwrite) const
{
  if (Analyze_ == 0) {
    ErrorMsg("No ANALYSIS_FILE set.\n");
    return 1;
  }
  Analyze_->Info();
  ChangeDir( TopDir );
  // Check that analysis directory, input, and script exist.
  std::string suffix(integerToString(start) + "." + integerToString(stop));
  std::string CPPDIR("Analyze." + suffix);
  if (CheckExists("analysis input directory", CPPDIR)) return 1;
  std::string inputName("batch.cpptraj.in"); // TODO make option
  if (CheckExists("analysis input file", CPPDIR + "/" + inputName)) return 1;
  std::string scriptName("RunAnalysis.sh"); // TODO make option
  if (CheckExists("analysis script", CPPDIR + "/" + scriptName)) return 1;

  // Set options specific to queuing system, node info, and Amber env.
  std::string qName( std::string(Analyze_->SubmitCmd()) + ".sh" );
  std::string qNamePath( CPPDIR + "/" + qName );
  if (!overwrite && fileExists( qNamePath )) {
    ErrorMsg("Not overwriting existing script %s\n", qNamePath.c_str());
    return 1;
  }
  TextFile qout;
  if (qout.OpenWrite( qNamePath )) return 1;
  if (Analyze_->QsubHeader(qout, -1, std::string(), "proc." + suffix + ".")) return 1;
  qout.Printf("\n# Run script\n./%s\nexit $?\n", scriptName.c_str());
  qout.Close();
  ChangePermissions( qNamePath );
  // Submit job
  if (testing_)
    Msg("Just testing; not submitting analysis job.\n");
  else {
    ChangeDir( CPPDIR );
    std::string submitCommand( std::string(Analyze_->SubmitCmd()) + " " + qName );
    if ( system( submitCommand.c_str() ) ) {
      ErrorMsg("Analysis job submission failed.\n");
      return 1;
    }
  }
  return 0;
}

int Submitter::SubmitArchive(std::string const& TopDir, int start, int stop, bool overwrite) const
{
  if (Archive_ == 0) {
    ErrorMsg("No ARCHIVE_FILE set.\n");
    return 1;
  }
  Archive_->Info();
  ChangeDir( TopDir );
  // TODO: Require or have option for dependency on analysis job.
  // Check that archive dir, input, and run script exist
  std::string suffix(integerToString(start) + "." + integerToString(stop));
  std::string ARDIR("Archive." + suffix);
  if (CheckExists("archive input directory", ARDIR)) return 1;
  std::string scriptName("RunArchive." + suffix + ".sh");
  if (CheckExists("archive run script", scriptName)) return 1;
  StrArray ar1_files = ExpandToFilenames(ARDIR + "/ar1.*.in");
  StrArray ar2_files = ExpandToFilenames(ARDIR + "/ar2.*.in");
  if (ar2_files.empty()) {
    ErrorMsg("No archive input found in %s\n", ARDIR.c_str());
    return 1;
  }
  if (!ar1_files.empty() && ar2_files.size() != ar1_files.size()) {
    ErrorMsg("Number of full archive input files %zu != number of stripped archive inputs %zu\n",
             ar1_files.size(), ar2_files.size());
    return 1;
  }

  // Set options specific to queuing system, node info, and Amber env.
  std::string qName("archive." + std::string(Archive_->SubmitCmd()) + "." + suffix + ".sh");
  if (!overwrite && fileExists( qName )) {
    ErrorMsg("Not overwriting existing script %s\n", qName.c_str());
    return 1;
  }
  TextFile qout;
  if (qout.OpenWrite( qName )) return 1;
  if (Archive_->QsubHeader(qout, -1, std::string(), "ar." + suffix + ".")) return 1;
  qout.Printf("\n# Run script\n./%s\nexit $?\n", scriptName.c_str());
  qout.Close();
  ChangePermissions( qName );
  // Submit job
  if (testing_)
    Msg("Just testing; not submitting archive job.\n");
  else {
    std::string submitCommand( std::string(Archive_->SubmitCmd()) + " " + qName );
    if ( system( submitCommand.c_str() ) ) {
      ErrorMsg("Archive job submission failed.\n");
      return 1;
    }
  }

  return 0;
}

int Submitter::ReadOptions(std::string const& fn) {
  Msg("  Reading queue options from '%s'\n", fn.c_str());
  n_input_read_ = 0;
  if (Run_ == 0) Run_ = new QueueOpts();
  if (ReadOptions( fn, *Run_ )) return 1;
  return 0;
}

int Submitter::CheckOptions() {
  if (Run_ == 0) return 1;
  if (Run_->Check()) return 1;
  Run_->CalcThreads();
  if (Analyze_ != 0) {
    if (Analyze_->Check()) return 1;
    Analyze_->CalcThreads();
  }
  if (Archive_ != 0) {
    if (Archive_->Check()) return 1;
    Archive_->CalcThreads();
  }
  return 0;
}

int Submitter::ReadOptions(std::string const& fnameIn, QueueOpts& Qopt) {
  n_input_read_++;
  if (n_input_read_ > 100) {
    ErrorMsg("# of input files read > 100; possible infinite recursion.\n");
    return 1;
  }

  if (debug_ > 0)
    Msg("  Reading queue options from '%s'\n", fnameIn.c_str());
  std::string fname = tildeExpansion( fnameIn );
  if (CheckExists( "Queue options", fname )) return 1;
  TextFile infile;
  OptArray Options = infile.GetOptionsArray(fname, debug_);
  if (Options.empty()) return 1;
  for (OptArray::const_iterator opair = Options.begin(); opair != Options.end(); ++opair)
  {
    std::string const& line = opair->first;
    std::string const& Args = opair->second;
    if (debug_ > 0)
      Msg("Opt: '%s'   Args: '%s'\n", line.c_str(), Args.c_str());

    // Process options
    if (line == "ANALYZE_FILE") {
      if (Analyze_ != 0) {
        ErrorMsg("Only one ANALYZE_FILE allowed.\n");
        return 1;
      }
      Analyze_ = new QueueOpts(Qopt);
      if (ReadOptions( Args, *Analyze_ )) return 1;
    } else if (line == "ARCHIVE_FILE") {
      if (Archive_ != 0) {
        ErrorMsg("Only one ARCHIVE_FILE allowed.\n");
        return 1;
      }
      Archive_ = new QueueOpts(Qopt);
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
  }

  return 0;
}
