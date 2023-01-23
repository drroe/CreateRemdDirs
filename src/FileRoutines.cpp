#include <cstdio>    // fopen, fclose
#include <cerrno>
#include <cstring>
#include <sys/stat.h> // mkdir
#include <unistd.h> // getcwd 
#ifndef __PGI
#  include <glob.h>  // For tilde expansion
#endif
#include "FileRoutines.h"
#include "Messages.h"

using namespace Messages;

// tildeExpansion()
/** Use glob.h to perform tilde expansion on a filename, returning the
  * expanded filename. If the file does not exist or globbing fails return an
  * empty string. Do not print an error message if the file does not exist
  * so that this routine and fileExists() can be used to silently check files.
  */
std::string FileRoutines::tildeExpansion(std::string const& filenameIn) {
  if (filenameIn.empty()) {
    ErrorMsg("tildeExpansion: null filename specified.\n");
    return std::string("");
  }
# ifdef __PGI
  // NOTE: It seems some PGI compilers do not function correctly when glob.h
  //       is included and large file flags are set. Just disable globbing
  //       for PGI and return a copy of filenameIn.
  // Since not globbing, check if file exists before returning filename.
  FILE *infile = fopen(filenameIn.c_str(), "rb");
  if (infile == 0) return std::string("");
  fclose(infile);
  return filenameIn;
# else
  glob_t globbuf;
  globbuf.gl_offs = 1;
  std::string returnFilename;
  int err = glob(filenameIn.c_str(), GLOB_TILDE, NULL, &globbuf);
  if ( err == GLOB_NOMATCH )
    //ErrorMsg("'%s' does not exist.\n", filenameIn); // Make silent
    return returnFilename;
  else if ( err != 0 )
    ErrorMsg("glob() failed for '%s' (%i)\n", filenameIn.c_str(), err);
  else {
    returnFilename.assign( globbuf.gl_pathv[0] );
    globfree(&globbuf);
  }
  return returnFilename;
# endif
}

// ExpandToFilenames()
/** Expand given expression with file name wildcards into an array of
  * strings. Optionally print warnings.
  */
FileRoutines::StrArray FileRoutines::ExpandToFilenames(std::string const& fnameArg, bool printWarnings) {
  StrArray fnames;
  if (fnameArg.empty()) return fnames;
# ifdef __PGI
  // NOTE: It seems some PGI compilers do not function correctly when glob.h
  //       is included and large file flags are set. Just disable globbing
  //       for PGI and return a copy of filenameIn.
  // Check for any wildcards in fnameArg
  if ( fnameArg.find_first_of("*?[]") != std::string::npos )
    Msg("Warning: Currently wildcards in filenames not supported with PGI compilers.\n");
  fnames.push_back( fnameArg );
# else
  glob_t globbuf;
  int err = glob(fnameArg.c_str(), GLOB_TILDE, NULL, &globbuf );
  //Msg("DEBUG: %s matches %zu files.\n", fnameArg.c_str(), (size_t)globbuf.gl_pathc);
  if ( err == 0 ) {
    for (unsigned int i = 0; i < (size_t)globbuf.gl_pathc; i++)
      fnames.push_back( globbuf.gl_pathv[i] );
  } else if (err == GLOB_NOMATCH ) {
    if (printWarnings)
      Msg("Warning: %s matches no files.\n", fnameArg.c_str());
  } else
    ErrorMsg("Problem occurred trying to find %s\n", fnameArg.c_str());
  if ( globbuf.gl_pathc > 0 ) globfree(&globbuf);
# endif
  return fnames;
}

/** Version of ExpandToFilenames that prints warnings. */
FileRoutines::StrArray FileRoutines::ExpandToFilenames(std::string const& fnameArg) {
  return ExpandToFilenames(fnameArg, true);
}

// fileExists()
/** \return true if file can be opened "r".  */
bool FileRoutines::fileExists(std::string const& filenameIn) {
  // Perform tilde expansion
  std::string fname = tildeExpansion(filenameIn);
  if (fname.empty()) return false;
  FILE *infile = fopen(fname.c_str(), "rb");
  if (infile==0) {
    ErrorMsg("File '%s': %s\n", fname.c_str(), strerror( errno ));
    return false;
  }
  fclose(infile);
  return true;
}

int FileRoutines::CheckExists(const char* type, std::string const& fname) {
  if (fname.empty() || !fileExists(fname)) {
    ErrorMsg("%s not found: '%s'\n", type, fname.c_str());
    return 1;
  }
  return 0;
}

/** \return 1 if file is a directory, 0 if not, -1 if error. */
int FileRoutines::IsDirectory(std::string const& filenameIn) {
  if (filenameIn.empty()) return -1;
  struct stat frame_stat;
  if (stat(filenameIn.c_str(), &frame_stat) == -1) {
    ErrorMsg( "Could not find file status for %s\n", filenameIn.c_str());
    //if (debug_>0)
    perror("     Error from stat: ");
    return -1;
  }
  if (frame_stat.st_mode & S_IFDIR)
    return 1;
  return 0;
}

int FileRoutines::Mkdir(std::string const& dname) {
  if (!fileExists(dname)) {
    //Msg("Creating directory '%s'\n", dname.c_str());
    if (mkdir( dname.c_str(), S_IRWXU ) != 0) {
      ErrorMsg("Creating dir '%s': %s\n", dname.c_str(), strerror( errno ));
      return 1;
    }
  } else
    Msg("Dir %s already present.\n", dname.c_str());
  return 0;
}

std::string FileRoutines::GetWorkingDir() {
  char buffer[1024];
  if (getcwd(buffer, 1024) == 0) {
    ErrorMsg("Getting current working dir name: %s\n", strerror( errno ));
    return std::string("");
  }
  return ( std::string(buffer) );
}

int FileRoutines::ChangeDir(std::string const& dname) {
  if (dname.empty()) {
    ErrorMsg("Cannot change dir; dir name is empty.\n");
    return 1;
  }
  //Msg("DEBUG: Changing to dir '%s'\n", dname.c_str());
  if (chdir( dname.c_str() ) != 0) {
    ErrorMsg("Changing to dir '%s': %s\n", dname.c_str(), strerror( errno ));
    return 1;
  }
  return 0;
}

int FileRoutines::ChangePermissions(std::string const& fname) {
  // For now only 775
  return chmod(fname.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

std::string FileRoutines::UserName() {
  char buffer[1024];
  FILE* fp = popen("whoami", "r");
  if (fp == 0) return std::string();
  char* ptr = fgets(buffer, 1023, fp);
  pclose( fp );
  if (ptr == 0) return std::string();
  return std::string(ptr);
}

/** \return portion of file name after final '.' */
std::string FileRoutines::Extension(std::string const& fname) {
  // Get the filename extension
  size_t found = fname.find_last_of(".");
  if (found == std::string::npos || found+1 == fname.size()) {
    return std::string("");
  }
  return fname.substr(found+1);
}

/** If given path is a relative path, add a '../' prefix. */
std::string FileRoutines::add_path_prefix(std::string const& path) {
  if (path.empty() || path[0] == '/')
    // No path or absolute path
    return path;
  else
    // Not an absolute path
    return std::string("../" + path);
}
