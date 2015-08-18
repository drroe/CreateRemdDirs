#include "Submit.h"
#include "Messages.h"
#include "TextFile.h"
#include "FileRoutines.h"


Submit::~Submit() {
  if (Run_ != 0) delete Run_;
  if (Analyze_ != 0) delete Analyze_;
  if (Archive_ != 0) delete Archive_;
}

int Submit::ReadOptions(std::string const& fn) {
  if (Run_ != 0) {
    ErrorMsg("Only one queue input allowed.\n");
    return 1;
  }
  Run_ = new QueueOpts();
  return ReadOptions( fn, *Run_ );
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

int Submit::QueueOpts::ProcessOption(std::string const& OPT, std::string const& VAR) {
  Msg("Processing '%s' '%s'\n", OPT.c_str(), VAR.c_str()); // DEBUG
  return 0;
}
