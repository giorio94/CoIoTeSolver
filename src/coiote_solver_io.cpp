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


#include <cmath>
#include <iostream>
#include <string>

#include "coiote_solver.h"

coiote_solver::coiote_solver(std::istream& input_file, const size_type& n_cells, const size_type& n_timesteps, const size_type& n_custtypes) :
	n_cells(n_cells), n_time_steps(n_timesteps), n_cust_types(n_custtypes),
	problem(n_cells, n_custtypes, n_timesteps), statistics(n_cells, n_custtypes, n_timesteps),
	has_solution(false), solution({ n_cells, n_cells, n_cust_types, n_time_steps }),
	time_finished(false), fewusers_time_finished(false) {

	// Read the number of activities done by each type of user
	for(size_type  m = 0; m < n_cust_types; m++) {
		input_file >> problem.act_per_user[m];;
	}

	// Read the matrix of costs
	unsigned tmp_i; std::string tmp_s;
	for(size_type m = 0; m < n_cust_types; m++) {
		for(size_type t = 0; t < n_time_steps; t++) {
			input_file >> tmp_i; // Read m index (useless)
			input_file >> tmp_i; // Read t index (useless)
			for(size_type i = 0; i < n_cells; i++)
				for(size_type j = 0; j < n_cells; j++) {
					// Read the costs as strings and then convert them into integer
					input_file >> tmp_s;
					problem.costs[{i,j,m,t}] = std::stoi(tmp_s);
				}
		}
	}

	// Read the activities to be done
	for(size_type i = 0; i < n_cells; i++) {
		input_file >> problem.activities[i];
	}

	// Read the number of users for each type and time step
	for(size_type m = 0; m < n_cust_types; m++) {
		for(size_type t = 0; t < n_time_steps; t++) {
			input_file >> tmp_i; // Read m index (useless)
			input_file >> tmp_i; // Read t index (useless)
			for(size_type i = 0; i < n_cells; i++)
				input_file >> problem.users_available[{i,m,t}];
		}
	}
}

void coiote_solver::write_kpi(std::ostream& output_file, const std::string& instance_name) {
	if(!has_solution)
		return;

	output_file << instance_name << ";" << kpi[0] << ";" << kpi[1];
	for(size_type i = 2; i < kpi.size(); i++)
		output_file <<  ";" << kpi[i];
	output_file << std::endl;
}

void coiote_solver::write_solution(std::ostream& solution_file) {
	if(!has_solution)
		return;

	solution_file << n_cells << ";" << n_time_steps << ";" << n_cust_types << std::endl;
	for(size_type m = 0; m < n_cust_types; m++)
		for(size_type t = 0; t < n_time_steps; t++)
			for(size_type i = 0; i < n_cells; i++)
				for(size_type j = 0; j < n_cells; j++)
					if(solution[{i,j,m,t}] > 0)
						solution_file << i << ";" << j << ";" << m << ";" << t << ";" << solution[{i,j,m,t}] << std::endl;
}

coiote_solver::feasibility_state coiote_solver::is_feasible() {
	// Handle the case no solution has been found
	if(!has_solution)
		return feasibility_state::NO_SOLUTION;

	const double eps = 0.001;
	const double objfun_value = kpi[0];
	double objfun_verify = 0; int counter;

	// Verify for each destination cell if the demand is satisfied
	for(size_type j = 0; j < n_cells; j++) {
		counter = 0;
		for(size_type i = 0; i < n_cells; i++)
			for(size_type m = 0; m < n_cust_types; m++)
				for(size_type t = 0; t < n_time_steps; t++) {
					counter += problem.act_per_user[m] * solution[{i,j,m,t}];
					// Recompute also the value of the objective funciton
					objfun_verify += solution[{i,j,m,t}] * problem.costs[{i,j,m,t}];
				}
		if(counter < problem.activities[j])
			return feasibility_state::NOT_FEASIBLE_DEMAND;
	}

	// Verify for each user type (i, m, t) that the number of users moved
	// does not exceed the number of available ones
	for(size_type i = 0; i < n_cells; i++)
		for(size_type m = 0; m < n_cust_types; m++)
			for(size_type t = 0; t < n_time_steps; t++) {
				counter = 0;
				for(size_type j = 0; j < n_cells; j++)
					counter += solution[{i,j,m,t}];
				if(counter > problem.users_available[{i,m,t}])
					return feasibility_state::NOT_FEASIBLE_USERS;
				// Check that no users do activities in their source cell
				if(solution[{i,i,m,t}] != 0)
					return feasibility_state::NOT_FEASIBLE_USERS;
			}

	// Compare the two computed objective function in order to verify the correctness
	if(fabs(objfun_verify - objfun_value) > eps)
		return feasibility_state::WRONG_OBJFUNCTVAL;

	// If no problem has been detected, the solution is feasible
	return feasibility_state::FEASIBLE;
}
