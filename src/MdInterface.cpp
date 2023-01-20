#include "MdInterface.h"
#include "Messages.h"
// ----- Packages ------------
#include "MdPackage_Amber.h"

using namespace Messages;

/** CONSTRUCTOR */
MdInterface::MdInterface() :
  package_(0),
  type_(UNKNOWN_PACKAGE)
{}

/** DESTRUCTOR */
MdInterface::~MdInterface() {
  if (package_ != 0) delete package_;
}

/** COPY CONSTRUCTOR */
MdInterface::MdInterface(MdInterface const& rhs) :
  package_(rhs.package_->Copy()),
  type_(rhs.type_)
{}

/** ASSIGNMENT */
MdInterface& MdInterface::operator=(MdInterface const& rhs) {
  if (&rhs == this) return *this;
  if (package_ != 0) delete package_;
  package_ = rhs.package_->Copy();
  type_ = rhs.type_;
  return *this;
}

/** Allocate package. */
int MdInterface::AllocatePackage(Type typeIn) {
  if (package_ != 0) delete package_;
  switch (typeIn) {
    case AMBER : package_ = new MdPackage_Amber(); break;
    case UNKNOWN_PACKAGE :
    default:
      ErrorMsg("Unhandled type in AllocatePackage()\n");
      return 1;
  }
  if (package_ == 0) {
    ErrorMsg("Could not allocate package.\n");
    return 1;
  }
  return 0;
}