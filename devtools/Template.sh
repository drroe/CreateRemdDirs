#!/bin/bash

# Create template for common classes

Help() {
  echo "Usage: $0 <name> [<type>]"
  echo "  <type>: Exec"
}

NAME=$1
if [ -z "$NAME" ] ; then
  echo "Enter name."
  Help
  exit 1
fi

TYPE=$2
if [ -z "$TYPE" ] ; then
  echo "Enter type."
  Help
  exit 1 
fi
if [ "$TYPE" != 'Exec' ] ; then
  echo "Type $TYPE not recognized."
  Help
  exit 1
fi

CLASS=$TYPE"_"$NAME
H_FILE=$CLASS".h"
C_FILE=$CLASS".cpp"
TYPEH="$TYPE"
echo "Creating class $CLASS in $H_FILE and $C_FILE"

# Check files
if [ -e "$H_FILE" -o -e "$C_FILE" ] ; then
  echo Already there.
  exit 1
fi

# Header protect
cat > $H_FILE <<EOF
#ifndef INC_${TYPE^^}_${NAME^^}_H
#define INC_${TYPE^^}_${NAME^^}_H
#include "$TYPEH.h"
/// <Enter description of $CLASS here>
class $CLASS : public $TYPEH {
EOF

# Class-specific header/implementation
# ----- Exec -------------------------------------
if [ "$TYPE" = 'Exec' ] ; then
  cat >> $H_FILE <<EOF
  public:
    /// CONSTRUCTOR
    $CLASS();
    void Help() const;
    RetType Execute(Manager&, Cols&) const;
};
#endif
EOF

  cat > $C_FILE <<EOF
#include "$H_FILE"
#include "Messages.h"

using namespace Messages;

/** CONSTRUCTOR */
$CLASS::$CLASS() {}

/** Help text */
void $CLASS::Help() const {
}

/** <Command description goes here.> */
Exec::RetType $CLASS::Execute(Manager& manager, Cols& args) const {
  return OK;
}
EOF
fi
