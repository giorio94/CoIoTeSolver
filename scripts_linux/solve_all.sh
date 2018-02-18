#!/bin/bash

# This bash script allows you to test the program with all the input files
# contained in the INDIR folder, comparing the obtained output against the
# optimal solutions provided by the teachers.


# This file is part of CoIoTeSolver.

# CoIoTeSolver is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# CoIoTeSolver is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with CoIoTeSolver. If not, see <http://www.gnu.org/licenses/>.


SCRDIR="$(dirname $0)"

EXE="$SCRDIR/../CoIoTeSolver.out"
INDIR="$SCRDIR/../input"
OUTDIR="$SCRDIR/../output"
SUMMARY="summary.csv"

CMPDIR="$SCRDIR/compare"
OPTSOL="$CMPDIR/optimal_solutions.csv"
COMPARE="$SCRDIR/compare.awk"
RESULT="$CMPDIR/result"
SUMMARY_BAK="$CMPDIR/summary"


if [ ! -e "$EXE" ]
then
	echo "Executable file $EXE not found - ABORT"
	exit -1
fi
if [ ! -d "$INDIR" ]
then
	echo "Input directory $INDIR not found - ABORT"
	exit -2
fi
if [ ! -d "$OUTDIR" ]
then
	mkdir -p "$OUTDIR"
fi

rm -f "$OUTDIR"/*
for FILENAME in "$INDIR"/*.txt
do
	echo -n "$(basename $FILENAME): "
	"./$EXE" "$FILENAME" "$OUTDIR/$SUMMARY" "$OUTDIR/$(basename $FILENAME)" --test
done


if [ ! -d "$CMPDIR" ]
then
	echo "Compare directory $CMPDIR not found - ABORT"
	exit -3
fi

if [ ! -e "$OPTSOL" ]
then
	echo "Optimal solutions file $OPTSOL not found - ABORT"
	exit -4
fi
if [ ! -e "$COMPARE" ]
then
	echo "Compare script $COMPARE not found - ABORT"
	exit -5
fi

DATE=$(date +%Y%m%d_%H%M%S)
"./$COMPARE" "$OUTDIR/$SUMMARY" "$OPTSOL" "$RESULT-$DATE"
cp "$OUTDIR/$SUMMARY" "$SUMMARY_BAK-$DATE"

