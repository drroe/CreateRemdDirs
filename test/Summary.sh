#!/bin/bash
SUMMARY='yes'
. MasterTest.sh

# Count the number of TEST_RESULTS files.
N_TESTS=`ls */$TEST_RESULTS 2> /dev/null | wc -l`

# Count the number of TEST_ERROR files.
N_ERR=`ls */$TEST_ERROR 2> /dev/null | wc -l`

echo "-----------------------------------------"
STAT=0
if [ $N_ERR -gt 0 ] ; then
  echo "$N_ERR of $N_TESTS tests failed."
  echo ""
  STAT=1
  for FILE in `ls */$TEST_ERROR` ; do
    echo "-----------------------------------------"
    echo "$FILE:"
    cat $FILE
    echo "-----------------------------------------"
  done
else
  echo "All $N_TESTS tests completed."
fi
echo "-----------------------------------------"
exit $STAT
