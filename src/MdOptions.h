#ifndef INC_MDOPTIONS_H
#define INC_MDOPTIONS_H
#include "Option.h"
class MdOptions {
  public:
    MdOptions();

    Option<double> const& TimeStep() const { return timeStep_; }
  private:
    Option<double> timeStep_;
};
#endif
