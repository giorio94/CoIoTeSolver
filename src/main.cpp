// This file is part of CoIoTeSolver.

// CoIoTeSolver is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// CoIoTeSolver is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with CoIoTeSolver. If not, see <http://www.gnu.org/licenses/>.


#include <string>
#include <iostream>
#include <fstream>

#include "coiote_solver.h"

void print_help(std::string exe_name);
void print_version();

int main(int argc, char **argv) {
	const unsigned time_limit_ms = 5000; // Maximum time allowed to solve the problem
	const int min_files = 2; // Minimum number of files accepted as parameters
	const int max_files = 3; // Maximum number of files accepted as parameters

	bool test = false;
	size_t nfiles = 0;
	std::string file_paths[max_files];

	// Iterate through all the imput parameters
	for(int i = 1; i < argc; i++) {
		std::string arg(argv[i]);

		// Print the help and exit
		if(arg == "--help" || arg == "-h") {
			print_help(argv[0]);
			return 0;
		}
		// Print the version and exit
		else if(arg == "--version") {
			print_version();
			return 0;
		}
		// Enable the feasibility test of the solution
		else if(arg == "--test")
			test = true;
		// Add the parameter to the file list
		else {
			if(nfiles >= max_files) {
				nfiles = max_files+1;
				break;
			}
			file_paths[nfiles++] = arg;
		}
	}

	// In case the number of files specified as parameters is wrong, abort the execution
	if(nfiles < min_files || nfiles > max_files) {
		print_help(argv[0]);
		return -1;
	}

	// Open the input stream to read the instance of the problem
	std::ifstream input_file(file_paths[0]);
	// Open the output stream (append mode) to save the KPIs of the solution
	std::ofstream output_file(file_paths[1], std::ios::app);

	// Check if both files have been opened correctly
	if(!input_file.is_open()) {
		std::cerr << "Impossible to open input file " << file_paths[0] << std::endl;
		return -2;
	}
	if(!output_file.is_open()) {
		std::cerr << "Impossible to open output file " << file_paths[1] << std::endl;
		return -3;
	}

	// Read from the input file the instance 'sizes'
	unsigned n_cells, n_timesteps, n_usertypes;
	input_file >> n_cells;
	input_file >> n_timesteps;
	input_file >> n_usertypes;

	// Initiate the solver class and close the input file
	coiote_solver solver(input_file, n_cells, n_timesteps, n_usertypes);
	input_file.close();

	// Do the real work: solve the problem
	solver.solve(time_limit_ms);

	// Write the KPIs to the output file after having got the instance file name as identifier
	std::string input_filename = file_paths[0].substr(file_paths[0].find_last_of("/\\") + 1);
	std::string instance_name = input_filename.substr(0, input_filename.find_last_of('.'));
	solver.write_kpi(output_file, instance_name);
	output_file.close();

	// In the case a file where writing the whole solution has been specified,
	// try to open it and then, if possible, save the solution
	if(nfiles == max_files) {
		std::ofstream solution_file(file_paths[2]);
		if(solution_file.is_open()) {
			solver.write_solution(solution_file);
			solution_file.close();
		}
		else
			std::cerr << "Impossible to open solution file " << file_paths[2] << std::endl;
	}

	// If the feasibility test has been enabled, execute it and then report the result
	if(test) {
		switch(solver.is_feasible()) {
			case coiote_solver::feasibility_state::FEASIBLE:
				std::cout << "Solution is feasible" << std::endl;
				break;
			case coiote_solver::feasibility_state::NOT_FEASIBLE_DEMAND:
				std::cout << "Solution is not feasible: demand not satisfied" << std::endl;
				break;
			case coiote_solver::feasibility_state::NOT_FEASIBLE_USERS:
				std::cout << "Solution is not feasible: exceeded number of available users" << std::endl;
				break;
			case coiote_solver::feasibility_state::WRONG_OBJFUNCTVAL:
				std::cout << "The objective function value is not computed correctly" << std::endl;
				break;
			case coiote_solver::feasibility_state::NO_SOLUTION:
				std::cout << "No solution found" << std::endl;
				break;
		}
	}

	return 0;
}

void print_help(std::string exe_name) {
	std::cerr << "Usage: " << exe_name << " [Options] InputFile OutputFile [SolutionFile]" << std::endl;
	std::cerr << " * InputFile: path of the input file describing the problem instance" << std::endl;
	std::cerr << " * OutputFile: path of the file to which append a summary of the solution" << std::endl;
	std::cerr << " * SolutionFile: path of the file where store the complete solution (optional)" << std::endl;
	std::cerr << "Options:" << std::endl;
	std::cerr << " * --test: parameter which enables some tests of correctness" << std::endl;
	std::cerr << " * --help: shows this help" << std::endl;
	std::cerr << " * --version: shows information about this program" << std::endl;
}

void print_version() {
	std::cerr << "CoIoTeSolver v2.3" << std::endl;
	std::cerr << std::endl;
	std::cerr << "CoIoTeSolver is free software: you can redistribute it and/or modify" << std::endl;
	std::cerr << "it under the terms of the GNU General Public License as published by" << std::endl;
	std::cerr << "the Free Software Foundation, either version 3 of the License, or" << std::endl;
	std::cerr << "(at your option) any later version." << std::endl;
	std::cerr << std::endl;
	std::cerr << "CoIoTeSolver is distributed in the hope that it will be useful," << std::endl;
	std::cerr << "but WITHOUT ANY WARRANTY; without even the implied warranty of" << std::endl;
	std::cerr << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the" << std::endl;
	std::cerr << "GNU General Public License for more details." << std::endl;
	std::cerr << std::endl;
	std::cerr << "You should have received a copy of the GNU General Public License" << std::endl;
	std::cerr << "along with CoIoTeSolver. If not, see <http://www.gnu.org/licenses/>." << std::endl;
}

