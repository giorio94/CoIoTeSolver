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


#include <algorithm>
#include <chrono>
#include <functional>
#include <limits>
#include <thread>

#include "coiote_solver.h"
#include "timer.h"

bool coiote_solver::solve(const unsigned long time_limit_ms) {
	// Start counting the elapsed time at the very beginning of the function
	auto start_time = std::chrono::steady_clock::now();

	const double perc_normal = 0.50; // Constant used to specify how much available time to use in case of a 'standard' instance
	const double perc_fewusers = 0.95; // Constant used to specify how much available time to use in case of a 'few users' instance
	const unsigned nthreads = 8; // Constant used to specify how many threads will be used

	const three_index_type three_dimensions = { n_cells, n_cust_types, n_time_steps };
	const four_index_type four_dimensions = { n_cells, n_cells, n_cust_types, n_time_steps };

	// Start the timers to manage the available time
	timer normal_timer((unsigned long)(time_limit_ms*perc_normal), [this](){ time_finished = true; });
	timer fewusers_timer((unsigned long)(time_limit_ms*perc_fewusers), [this](){ fewusers_time_finished = true; });

	// Generate the necessary statistics for the following computations (i.e. cost-based sorting)
	initialization_phase();

	double obj_function = std::numeric_limits<double>::infinity(); // Best objective function value found so far
	std::mt19937 rndgen; // Master random generator (a seed is not used in order to make it deterministic)

	std::array<th_parameter*, nthreads> parameters;
	std::array<std::thread, nthreads> threads;
	multi_array<int, 4>* best_solution = &solution;	// Pointer to the best solution found so far

	// Create one 'th_parameter' structure for each thread and then fire it
	for(size_type a = 0; a < nthreads; a++) {
		parameters[a] = new th_parameter(rndgen(), three_dimensions, four_dimensions);
		threads[a] = std::thread( &coiote_solver::thread_body, this, parameters[a] );
	}

	// Join again with all the threads and get the best solution found
	size_type iter_counter = 0;
	for(size_type a = 0; a < nthreads; a++) {
		threads[a].join();

		iter_counter += parameters[a]->iterations;
		if(parameters[a]->obj_function < obj_function) {
			obj_function = parameters[a]->obj_function;
			best_solution = &(parameters[a]->solution);
		}
	}

	// Store the best solution found
	solution = *best_solution;

	for(size_type a = 0; a < nthreads; a++) {
		delete(parameters[a]);
	}

	// Stop the timers
	normal_timer.stop();
	fewusers_timer.stop();

	// Handle the case of no feasible solution found
	if(obj_function == std::numeric_limits<double>::infinity()) {
		return (has_solution = false);
	}

	// Stop counting the elapsed time
	auto end_time = std::chrono::steady_clock::now();

	// Compute and store the KPIs (objective function, elapsed time, number of users for each type moved to another cell)
	kpi.push_back(obj_function);
	kpi.push_back(std::chrono::duration<double>(end_time-start_time).count());
	for(size_type m = 0; m < n_cust_types; m++) {
		unsigned n_users = 0;
		for(size_type i = 0; i < n_cells; i++)
			for(size_type j = 0; j < n_cells; j++)
				for(size_type t = 0; t < n_time_steps; t++)
					n_users += solution[{i,j,m,t}];
		kpi.push_back(n_users);
	}
	return (has_solution = true);
}

void coiote_solver::thread_body(th_parameter* const param) {
	const size_type iteration_limit = 10; // Constant used to specify how many iterations are done before trying to improve the solution

	multi_array<int, 3> users_available(param->three_dimensions); // Number of available users in each cell (used by the greedy function)
	multi_array<int, 4> current_solution(param->four_dimensions); // Current solution found through the greedy function
	multi_array<int, 4> best_solution(param->four_dimensions); // Local best solution found through the greedy function
	cells_usage usage(param->three_dimensions, problem.users_available); // Support structure to memorize the most 'chosen' users

	// Create a vector containing all the cells j to be visited
	std::vector<size_type> order;
	order.reserve(n_cells);
	for(size_type j = 0; j < n_cells; j++)
		if(problem.activities[j] > 0)
			order.push_back(j);

	// Define a function pointer in order to be able to change the greedy function if an instance with 'few users' is detected
	typedef double(coiote_solver::*greedy_function_type)(multi_array<int, 4>&,
		multi_array<int, 3>&, const std::vector<size_type>&, cells_usage&);
	greedy_function_type greedy_fn = &coiote_solver::greedy;
	bool few_users_mode = false;

	// Loop until there is enough time
	volatile bool* current_time_finished = &(this->time_finished);
	while(!(*current_time_finished)) {
		double best_objfun = std::numeric_limits<double>::infinity();
		size_type iterations = 0;

		// Loop iteration_limit times (if enough time is available)
		while(!(*current_time_finished) && iterations < iteration_limit) {

			// Generate a new randomic visiting order for the cells
			std::shuffle(order.begin(), order.end(), param->rndgen);

			// Execute the greedy function and update the local best solution if necessary
			double current_objfun;
			if((current_objfun = (this->*greedy_fn)(current_solution, users_available, order, usage)) < best_objfun) {
				best_objfun = current_objfun;
				best_solution = current_solution;
			}

			iterations++;

			// Handle the case of a 'few users' instance (the greedy has not been able to find a solution)
			if(current_objfun == std::numeric_limits<double>::infinity() && !few_users_mode) {
				// Create the necessary support structure
				if(statistics.act_slots == nullptr) {
					statistics.act_slots = new activities_slots(statistics.max_activities, n_cust_types, problem.act_per_user);
				}

				// Enter 'few users' mode changing the greedy function used and increasing the available time
				few_users_mode = true;
				current_time_finished = &(this->fewusers_time_finished);
				greedy_fn = &coiote_solver::greedy_few_users;
			}
		}
		param->iterations += iterations;

		// If the local best solution found by the greedy function is feasible, try to improve it
		if(best_objfun != std::numeric_limits<double>::infinity()) {
			double gain = -1;
			while(gain != 0 && !time_finished) {
				gain = improving_phase(best_solution);
				best_objfun -= gain;
			}
		}

		// Update the 'per thread' best solution found so far if necessary
		if(best_objfun < param->obj_function) {
			param->obj_function = best_objfun;
			param->solution = best_solution;
		}
	}
}

double coiote_solver::greedy(multi_array<int, 4>& solution, multi_array<int, 3>& users_available,
		const std::vector<size_type>& order, cells_usage& usage) {

	double obj_function = 0;

	solution.reset(); // Reset the solution to be built
	users_available = problem.users_available; // All the users are initially available

	vector_moves_type inserted_indexes; // Support vector to memorize all users moved to the current cell j
	four_index_type idx;

	// For each cell j to be visited (according to the current order)
	for(std::vector<size_type>::const_iterator it = order.begin(); it != order.end(); ++it) {
		const size_type j = *it;

		int demand = problem.activities[j];
		inserted_indexes.clear();

		// Until there is demand to be satisfied in the current cell
		while(demand > 0) {
			size_type min_i = 0, min_m = 0, min_t = 0;
			double cost, min_cost = std::numeric_limits<double>::infinity();

			// Get the cost-based index order to be used according to the remaining demand
			unsigned co_idx = statistics.get_costs_idx(demand);
			cells_order::const_iterator co_it = statistics.costs_order[co_idx][j].begin();
			cells_order::const_iterator co_end = statistics.costs_order[co_idx][j].end();

			// Loop according to not-decreasing costs until all users available have been considered
			while((co_it = statistics.costs_order[co_idx][j].get_least_expensive(co_it, users_available)) != co_end) {
				idx = *(co_it++);

				// Get the indexes and the cost (reduced by the number of activities) for each considered user
				size_type i = idx[four_index::i], m = idx[four_index::m], t = idx[four_index::t];
				cost = problem.costs[idx] / std::min(demand, problem.act_per_user[m]);

				// If the current cost is greater than the previous one stop iterating because no better choice is available
				if(cost > min_cost) {
					break;
				}

				// Replace the selected user with the current one if it is better (first iteration)
				// or if it could be convenient because in the previous greedy executions it was less used
				if(cost < min_cost || usage.should_replace({i,m,t}, {min_i,min_m,min_t})) {
						min_cost = cost;
						min_i = i;
						min_m = m;
						min_t = t;
				}
			}

			// No available users have been found to satisfy the current demand: impossible to continue
			if(min_cost == std::numeric_limits<double>::infinity()) {
				return min_cost;
			}

			// Compute the number of users to be assigned according to the availability and the need
			unsigned nusers = std::min(demand/problem.act_per_user[min_m], users_available[{min_i, min_m, min_t}]);
			if(nusers == 0) {
				nusers = 1;
			}

			idx = {min_i, j, min_m, min_t};
			solution[idx] += nusers; // Add the selected users to the solution
			obj_function += problem.costs[idx]*nusers; // Update the objective function value
			demand -= problem.act_per_user[min_m]*nusers; // Update the demand
			users_available[{min_i,min_m,min_t}] -= nusers; // Make the selected users no more available

			inserted_indexes.push_back(idx);
			usage.add({min_i,min_m,min_t}, nusers);
		}

		// In case more activities than necessary are done (it happens because users can do more than one task),
		// try to check if some of them (usually at most one) may be removed (the typical case is when at the beginning
		// users that can do few activities (e.g. one) are selected according to the cost-based order and at the end
		// users able to perform more tasks (e.g. three) are chosen because more convenient)
		if(demand < 0) {
			demand = -demand;

			// Sort the inserted users in the current cell j according to decreasing costs
			std::sort(inserted_indexes.begin(), inserted_indexes.end(), cmp_costs_desc(problem.costs));

			// Loop through them until there is an excess of activities done and remove the most
			// expensive users (if possible) updating at the same time the current solution
			vector_moves_type::const_iterator ins_idx_iter = inserted_indexes.begin();
			while(demand > 0 && ins_idx_iter != inserted_indexes.end()) {
				idx = *ins_idx_iter;
				if(problem.act_per_user[idx[four_index::m]] <= demand) {
					if((--solution[idx]) == 0) {
						++ins_idx_iter;
					}
					obj_function -= problem.costs[idx];
					demand -= problem.act_per_user[idx[2]];
					users_available[{idx[four_index::i], idx[four_index::m], idx[four_index::t]}]++;
				}
				else {
					++ins_idx_iter;
				}
			}
		}
	}

	return obj_function;
}

double coiote_solver::greedy_few_users(multi_array<int, 4>& solution, multi_array<int, 3>& users_available,
		const std::vector<size_type>& order, cells_usage& usage) {

	double obj_function = 0;

	solution.reset(); // Reset the solution to be built
	users_available = problem.users_available; // All the users are initially available
	four_index_type idx;

	// Generate a vector which for ach cell j to be visited associates the demand to be satisfied
	typedef std::vector<std::pair<size_type, int>> demand_type;
	demand_type remaining_demand(order.size());
	for(size_type b = 0; b < order.size(); b++)
		remaining_demand[b] = std::make_pair(order[b], problem.activities[order[b]]);

	// Loop two times: the first trying to move users in a conservative way (without wasting any activity)
	// and the second trying to satisfy all the remaining activities (enabling wasting)
	bool enable_wasting = false;
	for(size_type a = 0; a < 2; a++) {

		// For each cell j to be visited (according to the current order)
		for(size_type b = 0; b < remaining_demand.size(); b++) {
			const size_type j = remaining_demand[b].first;
			int demand = remaining_demand[b].second;

			// During the first iteration (enable_wasting = false) skip the current cell if it is compulsory to waste some activities
			if(!enable_wasting && statistics.act_slots->should_skip(demand)) {
				continue;
			}

			// Until there is demand to be satisfied in the current cell
			while(demand > 0) {
				size_type min_i = 0, min_m = 0, min_t = 0;
				double cost, min_cost = std::numeric_limits<double>::infinity();

				// Get the cost-based index order to be used according to the remaining demand
				unsigned co_idx = statistics.get_costs_idx(demand);
				cells_order::const_iterator co_it = statistics.costs_order[co_idx][j].begin();
				cells_order::const_iterator co_end = statistics.costs_order[co_idx][j].end();

				// Loop according to not-decreasing costs until all users available have been considered
				while((co_it = statistics.costs_order[co_idx][j].get_least_expensive(co_it, users_available)) != co_end) {
					idx = *(co_it++);

					// Get the indexes and the cost (reduced by the number of activities) for each considered user
					size_type i = idx[four_index::i], m = idx[four_index::m], t = idx[four_index::t];
					cost = problem.costs[idx] / std::min(demand, problem.act_per_user[m]);

					// If the current cost is greater than the previous one stop iterating because no better choice is available
					if(cost > min_cost) {
						break;
					}

					// Replace the selected user with the current one if more convenient or if it is able to perform more tasks
					// During the first global iteration (enable_wasting = false) only choices not leading to a waste of
					// activities can be done in order to maximize the probability to be able to find a feasible solution
					if((enable_wasting || statistics.act_slots->can_be_selected(demand, m)) &&
						(cost < min_cost || problem.act_per_user[m] > problem.act_per_user[min_m])) {
							min_cost = cost;
							min_i = i;
							min_m = m;
							min_t = t;
					}
				}

				// No available users have been found to satisfy the current demand
				if(min_cost == std::numeric_limits<double>::infinity()) {
					// If the iteration is already the final one (enable_wasting = true), then no feasible solution can be found
					if(enable_wasting) {
						return min_cost;
					}

					// Otherwise continue with the next cell, hoping the second iteration
					// will find some users by relaxing the 'no wasting' constraint
					break;
				}

				idx = {min_i, j, min_m, min_t};
				solution[idx]++; // Add the selected user to the solution
				obj_function += problem.costs[idx]; // Update the objective function value
				demand -= problem.act_per_user[min_m]; // Update the demand
				users_available[{min_i,min_m,min_t}]--; // Make the selected user no more available
			}

			remaining_demand[b] = std::make_pair(j, demand); // Update the remaining demand
		}
		enable_wasting = true; // Enable wasting (second phase)
	}

	return obj_function;
}

void coiote_solver::initialization_phase() {

	// Create a not-increasing sorted array containing the number of activities each user type can do
	for(size_type m = 0; m < n_cust_types; m++) {
		statistics.act_per_user_sorted[m] = problem.act_per_user[m];
	}
	std::sort(statistics.act_per_user_sorted, statistics.act_per_user_sorted+n_cust_types, std::greater<int>());
	statistics.max_act_per_user = statistics.act_per_user_sorted[0];

	// Create a number of threads equal to the number of user types, each one entitled to generate an array
	// of ordered indexes based on the cost per activity (depending on how much tasks that user type can do)
	std::vector<std::thread> threads;
	for(size_type m = 0; m < n_cust_types; m++)
		threads.push_back(std::thread( &coiote_solver::fill_cells_order, this, m ));

	// Get the maximum number of activities that must de done in one cell
	statistics.max_activities = 0;
	for(size_type j = 0; j < n_cells; j++)
		statistics.max_activities = std::max(statistics.max_activities, problem.activities[j]);

	// Wait all threads have terminated before continuing
	for(size_type m = 0; m < n_cust_types; m++)
		threads[m].join();
}

void coiote_solver::fill_cells_order(const size_type& index) {
	// For each cell j with a demand to be satisfied
	for(size_type j = 0; j < n_cells; j++) {

		// If the demand is zero it is not necessary to create the support structure for that cell
		if(problem.activities[j] == 0)
			continue;

		statistics.costs_order[index][j].initialize((n_cells-1)*n_cust_types*n_time_steps);
		// Loop through all the cells containing users (i, m, t), collecting the indexes
		for(size_type i = 0; i < n_cells; i++) {
			if(i == j) continue; // Users cannot do activities in their source cell
			for(size_type m = 0; m < n_cust_types; m++) {
				for(size_type t = 0; t < n_time_steps; t++) {
					// The index is collected only if there is at least one user in that cell
					if(problem.users_available[{i,m,t}] > 0) {
						statistics.costs_order[index][j].push_back({i,j,m,t});
					}
				}
			}
		}

		// Sort the indexes in a not-decreasing cost order, according to the comparator cmp_costs_asc
		statistics.costs_order[index][j].sort(
			(cmp_costs_asc(problem.costs, problem.act_per_user, statistics.act_per_user_sorted[index])));
	}
}

double coiote_solver::improving_phase(multi_array<int, 4>& solution) {
	moves_statistics statistics_moves = improving_setup(solution); // Generate the necessary support data structure

	double improvement = 0;
	// For each move (i, m, t -> j) in the current solution
	for(size_type a = 0; a < statistics_moves.moves.size() && !time_finished; a++) {
		// For each number of users between the maximum number of activities an user type can do and zero
		for(size_type m = statistics.max_act_per_user; m > 0 && !time_finished; m--) {
			ti_parameter param(statistics_moves.moves[a], m); // Create the parameter structure

			// Try to improve the current solution until it has success and there is enough time
			while(!time_finished && try_improve(solution, param, statistics_moves)) {
				// Update the current improvement in terms of objective function value
				for(size_type b = 0; b < param.imp_moves.size(); b++) {
					improvement	+= param.imp_moves[b].obj_gain;
				}
				param.clear();
			}
		}
	}

	return improvement;
}

coiote_solver::moves_statistics coiote_solver::improving_setup(multi_array<int, 4>& solution) {
	moves_statistics statistics_moves(n_cells, n_cust_types, n_time_steps);
	statistics_moves.users_available = problem.users_available; // Initialize the matrix of users available

	// For each element of the solution matrix
	for(size_type i = 0; i < n_cells; i++) {
		for(size_type j = 0; j < n_cells; j++) {
			if(i == j) continue; // If the source and destination cell are equal skip
			for(size_type m = 0; m < n_cust_types; m++) {
				for(size_type t = 0; t < n_time_steps; t++) {
					int x = solution[{i,j,m,t}]; // Get the number of users moved from i, m, t to j
					if(x == 0) continue; // If no users have been moved, go to the next element

					// Otherwise update the support structure, reducing the number of available users,
					// storing the current move in different vectors according to different criteria
					// (i.e. source or destination cell) and updating the number of activities done in
					// the current destination cell
					statistics_moves.users_available[{i,m,t}] -= x;
					statistics_moves.moves_from_i[i].push_back({i,j,m,t});
					statistics_moves.moves_to_j[j].push_back({i,j,m,t});
					statistics_moves.moves.push_back({i,j,m,t});
					statistics_moves.done_in_j[j]+=x*problem.act_per_user[m];
				}
			}
		}
	}

	return statistics_moves;
}

bool coiote_solver::try_improve(multi_array<int, 4>& solution, ti_parameter& param, moves_statistics& statistics_moves) {
	static const int min_gain = -4; // Constant used to specify the minimum gain allowed before stopping
	static const int max_level = 5; // Constant used to specify the maximum level of recursion
	static const int max_count = 20; // Constant used to specify the maximum number of iterations

	std::vector<improved_move> moves; // List of 'improving moves'

	// Indexes referred to the current cell to be modified
	const four_index_type curr_idx = param.curr_idx;
	size_type i = curr_idx[four_index::i], j = curr_idx[four_index::j];
	size_type m = curr_idx[four_index::m], t = curr_idx[four_index::t];

	// Abort the current recursion level if more users than the ones available in the solution should be removed,
	// if the maximum recursion level has been passed or if this cell is already in the 'tabu' list
	if(solution[curr_idx] < param.users_to_remove || param.it_level > max_level ||
		std::find(param.considered_cells.begin(), param.considered_cells.end(), curr_idx) != param.considered_cells.end()) {
		return false;
	}
	// Add the current cell to the 'tabu' list
	param.considered_cells.push_back(curr_idx);

	// Remove the decided number of users from the considered cell of the solution, updating the
	// objective function gain and the number of activities to be replaced
	double curr_gain = param.users_to_remove * problem.costs[curr_idx];
	int act_removed = problem.act_per_user[m] * param.users_to_remove;
	improved_move current_ic(i,j,m,t, -param.users_to_remove, -act_removed, curr_gain);
	param.obj_gain_so_far += add_remove_user(current_ic, solution, statistics_moves, false);
	moves.push_back(current_ic); // Add the current 'improving move' to the list

	// Get the cost-based index order to be used according to the number of activities to be replaced and the destination cell j
	unsigned co_idx = statistics.get_costs_idx(act_removed);
	cells_order::const_iterator co_it = statistics.costs_order[co_idx][j].begin();
	cells_order::const_iterator co_end = statistics.costs_order[co_idx][j].end();

	unsigned count = 0;
	// Loop according to not-decreasing costs until all users available have been considered
	while(co_it != co_end) {
		four_index_type new_idx = *(co_it++);
		size_type new_i = new_idx[four_index::i], new_m = new_idx[four_index::m], new_t = new_idx[four_index::t];

		// Compute the number of selected users to be added in order to perform the activities to be replaced
		int users_to_add = std::ceil((double)act_removed/problem.act_per_user[new_m]);

		// In case the considered index is already in the tabu list or if more users are needed than the number of them
		// available in the original problem in the given cell (i, m, t), skip and go to the next iteration
		if(std::find(param.considered_cells.begin(), param.considered_cells.end(), new_idx) != param.considered_cells.end() ||
			problem.users_available[{new_i,new_m,new_t}] < users_to_add) {
			continue;
		}
		unsigned prev_imp_size = moves.size();

		// Add the considered users to the solution, updating the objective function gain
		double curr_cost = problem.costs[new_idx] * users_to_add;
		improved_move current_ic(new_i, j, new_m, new_t, users_to_add, users_to_add*problem.act_per_user[new_m], -curr_cost);
		param.obj_gain_so_far += add_remove_user(current_ic, solution, statistics_moves, false);
		moves.push_back(current_ic); // Add the current 'improving move' to the list

		// Verify if it is possible to remove some previuosly inserted users because there is
		// some excess of activities done due to the different abilities of the types of users
		param.obj_gain_so_far += get_removable(j, solution, statistics_moves, moves);

		// Interrupt the search if the current gain is lower than the threshold, if the
		// number of iterations is above the limit or if the availabile time is finished
		if(param.obj_gain_so_far < min_gain || ++count > max_count || time_finished)
			break;

		// Compute the number of users considered in this iteration still available:
		// in case it is not negative, it means that the current solution is feasible
		int users_available = statistics_moves.users_available[{new_i,new_m,new_t}];
		if(users_available >= 0) {

			// In case the gain is positive, a better combination of users has been found and
			// it must be saved before returning with success to the previous recursion step
			if(param.obj_gain_so_far > 0) {
				param.imp_moves = moves;
				return true;
			}
			// Otherwise the changes done in the current iteration does not lead to an
			// improvement and so they must be undone before continuing with the next one
			else {
				for(size_type a = moves.size()-1; a >= prev_imp_size; a--) {
					param.obj_gain_so_far += add_remove_user(moves[a], solution, statistics_moves, true);
					moves.pop_back();
				}
				continue;
			}
		}

		// If the number of users considered in this iteration (with index: new_i, new_m, new_t)
		// still available is negative, the current solution would be not feasible, so try to
		// replace some tasks done by them in other destination cells by another recursion step
		for(size_type a = 0; a < statistics_moves.moves_from_i[new_i].size(); a++) {
			size_type dest_m = statistics_moves.moves_from_i[new_i][a][four_index::m];
			size_type dest_t = statistics_moves.moves_from_i[new_i][a][four_index::t];

			if(dest_m == new_m && dest_t == new_t) {
				// Build the parameter structure necessary for the next recursion step
				ti_parameter next(param, statistics_moves.moves_from_i[new_i][a], -users_available);
				// In case the next step has success, propagate the state by updating also the list of 'improving moves'
				if(try_improve(solution, next, statistics_moves)) {
					moves.insert(moves.end(), next.imp_moves.begin(), next.imp_moves.end());
					param.imp_moves = moves;
					return true;
				}
			}
		}

		// The changes done in the current iteration does not lead to a feasible
		// solution and so they must be undone before continuing with the next one
		for(size_type a = moves.size()-1; a >= prev_imp_size; a--) {
			param.obj_gain_so_far += add_remove_user(moves[a], solution, statistics_moves, true);
			moves.pop_back();
		}
	}

	// This recursion step has not been able to produce an improvement, so all changes are undone
	for(size_type a = 0; a < moves.size(); a++) {
		param.obj_gain_so_far += add_remove_user(moves[a], solution, statistics_moves, true);
	}
	// Remove the current cell from the 'tabu' list
	param.considered_cells.pop_back();
	return false;
}

double coiote_solver::add_remove_user(const improved_move& ic, multi_array<int, 4>& solution,
	moves_statistics& statistics_moves, const bool undo) {

	int flag = (undo) ? -1 : 1; // Flag depending whether the move must be done or undone

	solution[ic.f_idx] += (ic.user_added * flag); // Add to or remove from the solution the number of considered users
	statistics_moves.users_available[ic.t_idx] -= (ic.user_added * flag); // Update the number of users available
	statistics_moves.done_in_j[ic.f_idx[four_index::j]] += (ic.activities_added * flag); // Update the number of activities done
	return (ic.obj_gain * flag);
}

double coiote_solver::get_removable(const size_type j, multi_array<int, 4>& solution,
	moves_statistics& statistics_moves, std::vector<improved_move>& moves) {

	// Compute how many activities are done more than the necessary ones
	int redundancy = statistics_moves.done_in_j[j] - problem.activities[j];
	double gain = 0;

	// If there is some redundancy try to remove it in order to increase the gain
	if(redundancy > 0) {

		// Sort the users doing activities in the cell j according to not-increasing costs
		std::sort(statistics_moves.moves_to_j[j].begin(), statistics_moves.moves_to_j[j].end(), cmp_costs_desc(problem.costs));
		vector_moves_type::const_iterator ins_idx_iter = statistics_moves.moves_to_j[j].begin();

		// Loop through them until there is an excess of activities done and remove the
		// most expensive users (if possible) saving at the same time the 'improved move'
		while(redundancy > 0 && ins_idx_iter != statistics_moves.moves_to_j[j].end()) {
			four_index_type idx = *ins_idx_iter;
			if(problem.act_per_user[idx[four_index::m]] <= redundancy && solution[idx] > 0) {
				redundancy -= problem.act_per_user[idx[four_index::m]];
				improved_move current_ic(idx[four_index::i],idx[four_index::j],idx[four_index::m],idx[four_index::t],
					-1, -problem.act_per_user[idx[four_index::m]], problem.costs[idx]);
				moves.push_back(current_ic);
				gain += add_remove_user(current_ic, solution, statistics_moves, false);
			}
			else {
				++ins_idx_iter;
			}
		}
	}
	return gain;
}
