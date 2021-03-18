#include "System.h"

/** CONSTRUCTOR */
System::System() {}

/** CONSTRUCTOR - dirname, description */
System::System(std::string const& dirname, std::string const& description) :
  dirname_(dirname),
  description_(description)
{}

/** COPY CONSTRUCTOR */
System::System(System const& rhs) :
  dirname_(rhs.dirname_),
  description_(rhs.description_)
{}

/** Assignment */
System& System::operator=(System const& rhs) {
  if (this == &rhs) return *this;
  dirname_ = rhs.dirname_;
  description_ = rhs.description_;
  return *this;
}
