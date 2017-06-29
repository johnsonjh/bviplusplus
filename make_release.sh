#!/bin/bash

function usage  {
  echo "  Usage:"
  echo "      $0 <rel_version> [svn_version]"
  echo "      eg. $0 1.0 202"
  echo "      If no svn_version is supplied the current checked out version will be used."
}

if [ -z $1 ]; then
  usage
  return &> /dev/null || exit
else
  rev=$1
fi

if [ ! -z $2 ]; then
  rev_exists=`svn info -r $2 | grep "Revision:"`
  if [ -z "$rev_exists" ]; then
    echo "Revision $2 does not exists."
    usage
    return &> /dev/null || exit
  fi
  svnrev=$2
else
  svnrev=HEAD
fi

svn export -r $svnrev ./ ../bviplus-${rev}
cd ../
tar -czvf bviplus-${rev}.tgz bviplus-${rev}
rm -rf bviplus-${rev}
cd -

echo "Based on SVN rev #${svnrev}" > ../${rev}-relnotes.txt

