#include "Manager.h"
#include "Messages.h"
#include "TextFile.h"

using namespace Messages;

/** CONSTRUCTOR */
Manager::Manager() :
  debug_(0),
  activeProjectIdx_(0)
{}

/** Set debug levels. */
void Manager::SetDebug(int debugIn) {
  debug_ = debugIn;
}

/** Initialize with input file. */
int Manager::InitManager(std::string const& CurrentDir, std::string const& inputFileName) {
  if (inputFileName.empty()) {
    ErrorMsg("No manager input file given.\n");
    return 1;
  }
  topDir_ = CurrentDir;

  TextFile input;
  if (input.OpenRead(inputFileName)) {
    ErrorMsg("Could not open manager input file '%s'\n", inputFileName.c_str());
    return 1;
  }
  static const char* SEP = " \n\r";
  int ncols = input.GetColumns(SEP);
  while (ncols > -1) {
    if (ncols > 0) {
      if (input.Token(0)[0] != '#') {
        if ( input.Token(0) == "project" ) {
          // Expect project <name>
          if (ncols < 2) {
            std::string errline;
            for (int col = 1; col < ncols; col++)
              errline.append(" "+input.Token(col));
            ErrorMsg("Not enough columns for 'project': %s\n", errline.c_str());
            return 1;
          }
          // All columns beyond 0 are project name
          std::string description = input.Token(1);
          for (int col = 2; col < ncols; col++)
            description.append(" " + input.Token(col));
          Msg("Project: %s\n", description.c_str());
          projects_.push_back( Project(description) );
        } if ( input.Token(0) == "system" ) {
          // Expect system <system dir>, <description>
          if (ncols < 3) {
            std::string errline;
            for (int col = 1; col < ncols; col++)
              errline.append(" "+input.Token(col));
            ErrorMsg("Not enough columns for 'system': %s\n", errline.c_str());
            return 1;
          }
          // If no Project yet, add default
          if (projects_.empty())
            projects_.push_back( Project() );
          // All columns beyond the first are description
          std::string description = input.Token(2);
          for (int col = 3; col < ncols; col++)
            description.append(" " + input.Token(col));
          Msg("System: %s  Description: '%s'\n", input.Token(1).c_str(), description.c_str());
          projects_.back().AddSystem( System(CurrentDir, input.Token(1), description) );
          projects_.back().LastSystem().SetDebug( debug_ );
          if (projects_.back().LastSystem().FindRuns()) return 1;
        }
      }
    }
    ncols = input.GetColumns(SEP);
  }
  input.Close();
  // Change to the active systems directory
  if (ChangeToActiveSystemDir()) return 1;

  return 0;
}

/** Change to active systems directory. */
int Manager::ChangeToActiveSystemDir() const {
  if (!projects_.empty()) {
    Project const& activeProject = projects_[activeProjectIdx_];
    if (activeProject.ActiveSystemIdx() > -1) {
      System const& activeSystem = activeProject.Systems()[activeProject.ActiveSystemIdx()];
      if (activeSystem.ChangeToSystemDir()) {
        ErrorMsg("Change to active system directory failed.\n");
        return 1;
      }
    }
  }
  return 0;
}

/** Set active project and system. */
int Manager::SetActiveProjectSystem(int tgtProjectIdx, int tgtSystemIdx) {
  if (tgtProjectIdx < 0 || tgtSystemIdx < 0) {
    ErrorMsg("Must specify valid active project and system indices.\n");
    return 1;
  }

  if ((unsigned int)tgtProjectIdx >= projects_.size()) {
    ErrorMsg("Project index %i is out of range.\n", tgtProjectIdx);
    return 1;
  }
  activeProjectIdx_ = tgtProjectIdx;
  Project& activeProject = ActiveProject();

  if ((unsigned int)tgtSystemIdx >= activeProject.Systems().size()) {
    ErrorMsg("System index %i is out of range.\n", tgtSystemIdx);
    return 1;
  }
  activeProject.SetActiveSystem( tgtSystemIdx );

  return 0;
}

/** \return True if there is an active system to return. */
bool Manager::HasActiveProjectSystem() const {
  if (!projects_.empty()) {
    Project const& activeProject = projects_[activeProjectIdx_];
    if (activeProject.ActiveSystemIdx() > -1)
      return true;
  }
  return false;
}

/** \return True if any systems need to be saved. */
bool Manager::SystemsNeedSave() const {
  int n_needs_save = 0;
  for (ProjectArray::const_iterator project = projects_.begin();
                                    project != projects_.end(); ++project)
  {
    for (Project::SystemArray::const_iterator system = project->Systems().begin();
                                              system != project->Systems().end(); ++system)
    {
      if (system->NeedsSave()) {
        Msg("Needs save: ");
        system->PrintSummary();
        n_needs_save++;
      }
    }
  }
  return (n_needs_save > 0);
}
