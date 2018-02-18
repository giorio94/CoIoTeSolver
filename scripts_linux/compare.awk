#!/usr/bin/gawk -f

# This script, written in awk, allows you to test the results obtained by the
# program against the optimal solutions provided by the teachers, writing an
# easy report to be able to quantify 'how good' the solution is.


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


BEGIN {
	FS=";"

	if(ARGC != 4) {
		print "Usage: COMPARE INPUT OPTSOL OUTPUT"
		print " - COMPARE: this script"
		print " - INPUT: result file you wish to compare"
		print " - OPTSOL: file containing the optimal solutions"
		print " - OUTPUT: file where the comparison will be written"
		noend = 1
		exit -1
	}

	output = ARGV[--ARGC]

	while(getline < ARGV[ARGC-1]) {
		gsub(/.txt/, "")
		optimum[$1] = $3
	}

	ARGC--
	timelimit = 5
	thresh[0] = 0.1
	thresh[1] = 0.25
	thresh[2] = 0.5
	thresh[3] = 1
	thresh[4] = 2

	zeroerr = 0
	for(idx in thresh)
		errcnt[idx] = 0
}

{
	gsub(/.txt/, "")

	printf "%-20s", $1 > output
	if(!($1 in optimum)) {
		print "*** Not found in the optimal solutions file ***" > output
		next
	}

	timesum += $3
	printf "T%2s", (($3 < timelimit) ? "OK" : "!!") > output            # test if time limits are respected
	printf "  O%2s", (($2 >= optimum[$1]) ? "OK" : "!!") > output       # test if the current solution is not better than the optimal

	printf "  %3d", $2-optimum[$1] > output                             # compute the difference between the current solution and the optimal one
	errors[i] = 100*($2-optimum[$1])/optimum[$1]                        # compute the error in percentage
	errorsum += errors[i]
	if(errormax < errors[i])
		errormax = errors[i]

	printf "  %6.3f", errors[i] > output

	printf "  " > output
	if($2-optimum[$1] == 0)
		zeroerr++;
	else
		printf "*" > output
	for(idx in thresh)
		if(errors[i] >= thresh[idx]) {
			errcnt[idx]++
			printf "*" > output
	}

	printf "\n" > output

	++i
}

END {
	if(noend == 0 && i > 0) {
		print    "" > output
		print "Total number of records: " i > output
		print "Number of records with error = 0: " zeroerr > output
		for(idx in thresh)
			print "Number of records with error >= " thresh[idx] ": " errcnt[idx] > output

		print "" > output
		printf "Average error: %7.4f\n", errorsum/i > output
		n = asort(errors)
		if(n % 2 == 0)
			median = (errors[n/2] + errors[n/2 +1])/2
		else
			median = errors[int(n/2)+1]
		printf "Median error:  %6.3f\n", median > output
		printf "Maximum error: %6.3f\n", errormax > output
		print "" > output
		printf "Average time:  %6.3f\n", timesum/i > output
	}
}
