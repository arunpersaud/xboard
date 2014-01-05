#!/bin/bash
#  lng2po.sh -- translating .lng files to .po files for XBoard/Winboard,
#               part of XBoard GNU project
#
#  Copyright 2011, 2013, 2014 Free Software Foundation, Inc.
#  ------------------------------------------------------------------------
#
#  GNU XBoard is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or (at
#  your option) any later version.
#
#  GNU XBoard is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program. If not, see http://www.gnu.org/licenses/.  *
#
# ------------------------------------------------------------------------


INFO="lng2po.sh (translating .lng files to .po files for XBoard/Winboard)"
USAGE="Usage: `basename $0` [-hv] [-o outputfile] [-e encoding] <input.lng>
               default encoding:    latin-1
               default output file: ./language.po"

VERSION="1"

# default options
OUTPUT="language.po"
ENCODING="latin-1"
INPUT=""
TMPFILE=`mktemp lng2poXXXXX`

# parse command line arguments
while getopts hvo:e: OPT; do
    case "$OPT" in
        h)  echo $USAGE
            exit 0
            ;;
        v)  echo "`basename $0` version: $VERSION"
            exit 0
            ;;
        o)  OUTPUT=$OPTARG
            ;;
        e)  ENCODING=$OPTARG
            ;;
        \?) # getopts issues an error message
            echo $USAGE >&2
            exit 1
            ;;
	:) echo " Error:options -e and -o need an argument"
	    exit 2
	    ;;
    esac
done

# move past the parsed commands in $@
shift `expr $OPTIND - 1`

# check if an input file was given and if it exists in ./ or winboard/languages
# also check if it's a .lng file
if [ $# -ne 1 ]; then
    echo "  Error: Need to specify an input file"
    exit 3
else
    if [ ${1: -4} == ".lng" ]; then
	INPUT=$1
    else
	echo " Error: Input file needs to be a .lng file"
	exit 4
    fi
fi

if [ -e ../winboard/language/$INPUT ]; then
    INPUT=../winboard/language/$INPUT
fi

if [ -e $INPUT ]; then
   # do the conversion

   cp $INPUT $TMPFILE
   recode $ENCODING $TMPFILE
   ed $TMPFILE < metascript
   rm $TMPFILE
   cp xboard.pot $OUTPUT
   ed $OUTPUT < lng2po-helper-script
   rm lng2po-helper-script
   echo "created $OUTPUT"
else
    echo " Error: Couldn't find input file ($INPUT) in current directory or winboard/languages"
    exit 5
fi

