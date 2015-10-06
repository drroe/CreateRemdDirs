# Source this file.

CLEANONLY=0

CleanFiles() {
  while [[ ! -z $1 ]] ; do
    if [[ -f "$1" ]] ; then
      rm $1
    elif [[ -d "$1" ]] ; then
      rm -rf $1
    fi
    shift
  done
  if [[ $CLEANONLY -eq 1 ]] ; then
    exit 0
  fi
}

RunTest() {
  if [[ -z $OPTLINE ]] ; then
    echo "Error: OPTLINE not set" > /dev/stderr
    exit 1
  fi
  echo "  Test: $1"
  echo "  Test: $1" >> $TEST_RESULTS
  $VALGRIND $BIN $OPTLINE >> $OUTPUT 2>> $ERROR
  if [[ $? -ne 0 ]] ; then
    echo "Program error." > $TEST_ERROR
    exit 1
  fi
}

DoTest() {
  ((NUMTEST++))
  if [[ ! -f "$1" ]] ; then
    echo "Error: Save file $1 missing." >> $TEST_ERROR
    echo "Error: Save file $1 missing." > /dev/stderr
    exit 1
  fi
  if [[ ! -f "$2" ]] ; then
    echo "Error: Test file $2 missing." >> $TEST_ERROR
    echo "Error: Test file $2 missing." > /dev/stderr
    exit 1
  fi
  $DIFFCMD $1 $2 > temp.diff
  if [[ -s "temp.diff" ]] ; then
    echo "  $1 $2 are different." >> $TEST_RESULTS
    echo "  $1 $2 are different." >> $TEST_ERROR
    cat temp.diff >> $TEST_ERROR
    ((ERR++))
  else
    echo "  $2 OK." >> $TEST_RESULTS
  fi
  rm temp.diff
}

EndTest() {
  if [[ $ERR -gt 0 ]] ; then
    echo "  $ERR out of $NUMTEST comparisons failed."
    echo "  $ERR out of $NUMTEST comparisons failed." >> $TEST_RESULTS
    echo "  $ERR out of $NUMTEST comparisons failed." >> $TEST_ERROR
  else
    echo "All $NUMTEST comparisons passed." 
    echo "All $NUMTEST comparisons passed." >> $TEST_RESULTS
  fi
  echo ""
  if [[ ! -z $VALGRIND ]] ; then
    echo "Valgrind summary:"
    grep ERROR $ERROR
    grep heap $ERROR
    grep LEAK $ERROR
    echo ""
    echo "Valgrind summary:" >> $TEST_RESULTS
    grep ERROR $ERROR >> $TEST_RESULTS
    grep heap $ERROR >> $TEST_RESULTS
    grep LEAK $ERROR >> $TEST_RESULTS
  fi
  exit $ERR
}

# ------------------------------------------------------------------------------
ERR=0
NUMTEST=0
BIN=../../bin/CreateRemdDirs
VALGRIND=""
ERROR=/dev/stderr
while [[ ! -z $1 ]] ; do
  case "$1" in
    "clean" ) CLEANONLY=1 ;;
    "vg"    )
      echo "Using valgrind."
      VALGRIND="valgrind --tool=memcheck --leak-check=yes --show-reachable=yes"
      ERROR="valgrind.out"
      if [[ -e $ERROR ]] ; then
        rm $ERROR
      fi
      ;;
    *       ) echo "Unrecognized option: $1" 2> /dev/stderr ; exit 1 ;;
  esac
  shift
done

# TODO Determine Binary location

if [[ ! -f "$BIN" ]] ; then
  echo "$BIN not found."
  exit 1
fi

DIFFCMD=`which diff`
if [[ -z $DIFFCMD ]] ; then
  echo "diff command not found."
  exit 1
fi

TEST_RESULTS=Test_Results.dat
if [[ -e $TEST_RESULTS ]] ; then
  rm $TEST_RESULTS
fi
TEST_ERROR=Test_Error.dat
if [[ -e $TEST_ERROR ]] ; then
  rm $TEST_ERROR
fi
OUTPUT=test.out
if [[ -e $OUTPUT ]] ; then
  rm $OUTPUT
fi
