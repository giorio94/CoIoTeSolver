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


#ifndef COIOTE_SOLVER_H
#define COIOTE_SOLVER_H

#include <array>
#include <random>
#include <vector>

#include "multi_array.h"
#include "activities_slots.h"
#include "cells_order.h"


/**
 * \brief Core class of CoIoTeSolver.
 *
 * This class can be considered the heart of the whole project. It provides
 * function and data structures to manage every aspect of the problem:
 * - Input/output methods that read the input files, save the information in
 * suitable data structures, and write out the final results.
 * - Data structures needed to store input data, statistical data,
 * partial and final results.
 * - Methods whose purpose is to manage some specific aspects of the generation
 * of the solution, from the one implementing the greedy algorithm to functions
 * devoted to the improving of the solution or the computation of some significant
 * information.
**/
class coiote_solver {

private:
	/** \brief size_type is defined as an alias of size_t. An unsigned integral type. **/
	typedef size_t size_type;

	/** \brief three_index_type represents the correct type to specify an element of a
	 * three dimensional array implemented through multi_array. **/
	typedef multi_array<int, 3>::index_type three_index_type;
	/** \brief four_index_type represents the correct type to specify an element of a
	 * four dimensional array implemented through multi_array. **/
	typedef multi_array<int, 4>::index_type four_index_type;

	/** \brief vector_moves_type represents the chosen data structure to contain a series
	 * of moves (i.e. visited cells or single elements of the solution) **/
	typedef std::vector<four_index_type> vector_moves_type;

	/**
	 * \brief This accessory structure provides a mnemonic way to access the
	 * different indexes contained inside an element of three_index_type, without having
	 * to remember their position.
	**/
	struct three_index {
		static const size_type i = 0; /**< \brief Source cell. **/
		static const size_type m = 1; /**< \brief User type. **/
		static const size_type t = 2; /**< \brief Time period. **/
	};
	/**
	 * \brief This accessory structure provides a mnemonic way to access the
	 * different indexes contained inside an element of four_index_type, without having
	 * to remember their position.
	**/
	struct four_index {
		static const size_type i = 0; /**< \brief Source cell. **/
		static const size_type j = 1; /**< \brief Destination cell. **/
		static const size_type m = 2; /**< \brief User type. **/
		static const size_type t = 3; /**< \brief Time period. **/
	};

public:
	/** \brief An enumeration used to describe in a mnemonic way the feasibility state of a solution. **/
	enum class feasibility_state {
		FEASIBLE, /**< \brief The solution is feasible. **/
		NOT_FEASIBLE_DEMAND, /**< \brief The demand is not satisfied in some cells. **/
		NOT_FEASIBLE_USERS, /**< \brief The amount of moved users exceedes the number of available ones. **/
		WRONG_OBJFUNCTVAL, /**< \brief The computed objective function is not correct. **/
		NO_SOLUTION /**< No solution has been found. **/
	};

	/**
	 * \brief Constructor.
	 *
	 * The constructor creates the coiote_solver object given the problem instance.
	 *
	 * \param input_file the stream linked to the instance file. The first line, providing the dimensions
	 * problem has to be already read. No check are done about the correctness of the file: in the case
	 * it does not fulfill the standard, the behavior of this and other methods is completely undefined.
	 * \param n_cells number of cells in the current instance file.
	 * \param n_custtypes number of different customer types in the current instance file.
	 * \param n_timesteps number of different time periods in the current instance file.
	**/
	coiote_solver(std::istream& input_file, const size_type& n_cells, const size_type& n_custtypes, const size_type& n_timesteps);

	/**
	 * \brief Tries to solve the problem.
	 *
	 * This method uses up to the time specified as parameter in order to find
	 * the best solution (closest to the optimal one) as possible to the given
	 * problem.
	 *
	 * It uses a mix of different tecniques, actually calling other private methods
	 * implementing different steps of the solution generation, which have shown to provide
	 * very good results in the case of the instances provided to us to test our code.
	 *
	 * In the case the result is not as expected, in this specific function and in other
	 * methods, it is possible to tune some simple parameters (e.g. the fraction of available
	 * time actually used or the number of threads generated) in order to adapt it to
	 * the current environment and instance type.
	 *
	 * \param time_limit_ms the maximum time in milliseconds that the method can use to produce a solution.
	 * \return a boolean variable reporting if the method has been able to find a feasible solution or not.
	**/
	bool solve(const unsigned long time_limit_ms);

	/**
	 * \brief Writes some KPIs related to the solution on the output stream.
	 * \param output_file the stream linked to the file where writing such information.
	 * \param instance_name the identifier of the current instance.
	**/
	void write_kpi(std::ostream& output_file, const std::string& instance_name);

	/**
	 * \brief Writes the whole solution on the output stream.
	 * \param solution_file the stream linked to the file where writing such information.
	**/
	void write_solution(std::ostream& solution_file);

	/**
	 * \brief Performs some feasibility tests and reports the result.
	 * \return verdict of the test.
	**/
	feasibility_state is_feasible();

private:

	/** \brief Data structure containing all the relevant information read from the input file. **/
	struct input_problem {

		/** \brief costs to move a user of the given type from one cell to another in the specified time period. **/
		multi_array<double, 4> costs;

		/** \brief number of users available for each source cell, customer type and time period. **/
		multi_array<int, 3> users_available;

		/** \brief number of activities to be done in each cell. **/
		int* activities;

		/** \brief number of activities each user type is able to perform. **/
		int* act_per_user;

		/**
		 * \brief Constructor.
		 *
		 * Builds the different data structures contained according to the instance dimensions
		 * specified as parameters.
		 *
		 * \param n_cells number of cells.
		 * \param n_cust_types number of different customer types.
		 * \param n_time_steps number of different time periods.
		**/
		input_problem(const size_type& n_cells, const size_type& n_cust_types, const size_type& n_time_steps)
			: costs({n_cells, n_cells, n_cust_types, n_time_steps}), users_available({n_cells, n_cust_types, n_time_steps}) {
				act_per_user = new int[n_cust_types];
				activities = new int[n_cells];
		}

		/**
		 * \brief Destructor.
		**/
		~input_problem() {
			delete[](act_per_user);
			delete[](activities);
		}
	};

	/** \brief Data structure storing important statistics necessary to generate the solution efficently. **/
	struct global_statistics {

		/** \brief number of different customer types. **/
		const size_type n_cust_types;

		/**
		 * \brief matrix providing access to ordered costs.
		 *
		 * This matrix stores for each customers type (first index)
		 * and destination cell (second index) an array of indexes sorted
		 * by not-decreasing cost order.
		 *
		 * \see get_costs_idx()
		 * \see initialization_phase()
		 * \see coiote_solver::cmp_costs_asc
		**/
		cells_order** costs_order;

		/** \brief number of activities each user type is able to perform sorted in not-increasing order. **/
		int* act_per_user_sorted;

		/** \brief maximum number of activities that can be done by the users. **/
		int max_act_per_user;

		/** \brief maximum number of activities to be done. **/
		int max_activities;

		/** \brief slots of activities, computed and used only in case
		 * of instances with few users. **/
		activities_slots* act_slots;

		/**
		 * \brief Constructor.
		 *
		 * Builds the different data structures according to the instance dimensions
		 * specified as parameters.
		 *
		 * \param n_cells number of cells.
		 * \param n_cust_types number of different customer types.
		 * \param n_time_steps number of different time periods.
		**/
		global_statistics(const size_type& n_cells, const size_type& n_cust_types, const size_type& n_time_steps)
			: n_cust_types(n_cust_types) {
				act_per_user_sorted = new int[n_cust_types];
				costs_order = new cells_order*[n_cust_types];
				for(size_type i = 0; i < n_cust_types; i++)
					costs_order[i] = new cells_order[n_cells];
				act_slots = nullptr;
		}

		/**
		 * \brief Destructor.
		**/
		~global_statistics() {
			for(size_type i = 0; i < n_cust_types; i++)
				delete[](costs_order[i]);
			delete[](costs_order);
			delete[](act_per_user_sorted);
			delete(act_slots);
		}

		/**
		 * \brief Computes the index to be used for the first dimension of the costs_order matrix.
		 *
		 * The index is computed by considering the remaining demand to be satisfied in the
		 * current destination cell, and retrieving the one referring the order relative to the
		 * users type able to do the maximum number of activities not exceeding the demand.
		 *
		 * Be careful that this does not mean that the given order contains only users belonging
		 * to that type, but it implies that the reduced costs are computed considering at most
		 * that number of activities to be done.
		 *
		 * \param demand remaining demand in the current cell.
		**/
		inline unsigned get_costs_idx(const int demand) {
			unsigned m = 0;
			while(act_per_user_sorted[m] > demand && m < n_cust_types-1)
				++m;
			return m;
		}
	};

	struct moves_statistics;
	struct improved_move;
	struct th_parameter;
	struct ti_parameter;
	class cells_usage;
	class cmp_costs_desc;
	class cmp_costs_asc;


	const size_type n_cells; /**< \brief The number of cells in the current instance file. **/
	const size_type n_time_steps; /**< \brief The number of different time periods. **/
	const size_type n_cust_types; /**< \brief The number of different types of customers. **/

	input_problem problem; /**< \brief Input data relative to the current instance file. **/
	global_statistics statistics; /**< \brief Statistics relative to the current instance file. **/

	/** \brief A boolean variable specifying whether a feasible solution has been found or not. **/
	bool has_solution;

	/** \brief Multi-dimensional array used to store the best solution found. **/
	multi_array<int, 4> solution;

	/** \brief Vector containing some KPIs relative to the best solution found. **/
	std::vector<double> kpi;

	/** \brief A flag set to true when the time available to generate the solution is finished. **/
	volatile bool time_finished;
	/** \brief A flag set to true when the time available to generate the solution is finished,
	 * used in the case of instances with a very limited amount of users **/
	volatile bool fewusers_time_finished;

	/**
	 * \brief Builds up the necessary statistics, in particular the cost ordering through
	 * the function fill_cells_order.
	**/
	void initialization_phase();

	/**
	 * \brief Computes the cost ordering for the given user type.
	 *
	 * The ordering is done according to the cmp_costs_asc comparator, in particular the costs
	 * to be sorted are reduced by a factor which is the minimum between the number of activities
	 * the given user type can do and a limit corresponding to the number of tasks the user
	 * type specified as parameter is able to perform. Such a limit, and in particular the
	 * computation of different orders, is done to be able to consider the correct sequence also
	 * when only few activities have to be performed.
	 *
	 * \param index index of the limiting users type.
	**/
	void fill_cells_order(const size_type& index);

	/**
	 * \brief Tries to generate the solution.
	 *
	 * This function, executed ones per each thread fired, combines different strategies, in particular
	 * the repetition of the greedy function with different visiting orders interleaved with the
	 * improving_phase method, in order to get a solution as close as possible to the optimal one.
	 *
	 * It also detects the case of few users available (when the greedy function is not able to provide
	 * a solution) and modifies the solution generation strategy to overcome this problem.
	 *
	 * \param param contains all the necessary information necessary to do the computation and return
	 * the result.
	**/
	void thread_body(th_parameter* const param);


	/**
	 * \brief Greedy function which is the core of the solution generation.
	 *
	 * This function builds the solution by steps, considering one by one the destination cells
	 * where some activities have to be performed in the order specified by the parameter, and
	 * satisfying the tasks of each one using the most convenient users (considering the reduced
	 * costs) at the given moment.
	 *
	 * \param solution the data structure where the constructed solution is memorized. It is
	 * reset at the beginning of the method.
	 * \param users_available the data structure updated after each step and used to memorize
	 * the users still available. It is reset at the beginning of the method and it is passed
	 * as parameter, even if used only locally, in order to avoid multiple costly allocations.
	 * \param order order to be followed to visit the destination cells and satisfy the activities.
	 * \param usage a sort of picture of the previous invocations of this method, in particular
	 * related to the most often chosen groups of users.
	 * \return the objective function value relative to the current solution. It is equal to
	 * std::numeric_limits<double>::infinity() in the case no solution is found.
	**/
	double greedy(multi_array<int, 4>& solution, multi_array<int, 3>& users_available,
		const std::vector<size_type>& order, cells_usage& usage);

	/**
	 * \brief Modified version of the greedy function, used in the case of instances
	 * with a limited number of users in surplus.
	 *
	 * This function works in a similar way to the standard and more efficient greedy function,
	 * but is caracterized by a two step procedure, both exploiting the same logic of the
	 * other already mentioned function. In the first iteration, anyway, the method avoids to
	 * choose users which would eventually lead to a waste of activities (i.e. more activities
	 * done than the number of requests), in order to satisfy the constraints even if there are
	 * only few users in surplus. In the second one, on the other hand, this additional constraint
	 * is relaxed hoping to be able to conclude the remaining activities.
	 *
	 * \param solution the data structure where the constructed solution is memorized. It is
	 * reset at the beginning of the method.
	 * \param users_available the data structure updated after each step and used to memorize
	 * the users still available. It is reset at the beginning of the method and it is passed
	 * as parameter, even if used only locally, in order to avoid multiple costly allocations.
	 * \param order order to be followed to visit the destination cells and satisfy the activities.
	 * \param usage a sort of picture of the previous invocations of this method, in particular
	 * related to the most often chosen groups of users.
	 * \return the objective function value relative to the current solution. It is equal to
	 * std::numeric_limits<double>::infinity() in the case no solution is found.
	 *
	 * \see greedy()
	**/
	double greedy_few_users(multi_array<int, 4>& solution, multi_array<int, 3>& users_available,
		const std::vector<size_type>& order, cells_usage& usage);

	/**
	 * \brief Tries to improve the current solution.
	 *
	 * This method is entitled to generate the necessary statistics and to apply the
	 * try_improve method to the different moves composing the solution.
	 *
	 * \param solution current solution to try to improve.
	 * \return objective function value gain obtained.
	**/
	double improving_phase(multi_array<int, 4>& solution);

	/**
	 * \brief Computes the moves statistics starting from a solution already generated in order to
	 * be able to apply the try_improve method to improve it.
	 * \param solution the solution to be improved and used to generate the statistics.
	 * \return the generated data structure.
	**/
	moves_statistics improving_setup(multi_array<int, 4>& solution);

	/**
	 * \brief Recursive function which tries to improve the current solution.
	 *
	 * The main purpose of this method is to try to find a chain of changes (i.e. modifications
	 * about which groups of users perform the requested tasks) which as a whole leads to a
	 * smaller value of the objective function. In particular, starting from an already feasible
	 * solution, the function removes one or more users doing activities in a given destination
	 * cell and then tries to find other users which are able to perform better. In the case
	 * the selected customers are available, the recursion terminates with a positive result
	 * if the combination leads to an improvement. If the users are not available, on the other
	 * hand, the method checks whether is possible to replace some other activities done by
	 * the chosen users in other destination cells through a recursive call of the function.
	 *
	 * \param solution current solution to be improved.
	 * \param param a series of information necessary for the current iteration (in particular
	 * which element of the solution has to be modified and how many users have to be removed).
	 * \param statistics_moves statistics related to the current solution.
	 * \return a boolean value indicating whether the process outcome is positive or not.
	**/
	bool try_improve(multi_array<int, 4>& solution, ti_parameter& param, moves_statistics& statistics_moves);

	/**
	 * \brief Checks whether one or more users may be removed.
	 *
	 * Function used by the try_improve method to verify if, after having done
	 * some changes, more activities than necessary are done. In this case the
	 * most expensive users (compatibly with the constraints) are removed.
	 *
	 * \param j destination cell to be checked.
	 * \param solution current solution.
	 * \param statistics_moves data structures containing information related to the solution.
	 * \param moves array where the changes done will be recorded.
	 * \return objective function gain due to the changes.
	**/
	double get_removable(const size_type j, multi_array<int, 4>& solution, moves_statistics& statistics_moves, std::vector<improved_move>& moves);

	/**
	 * \brief Does or undoes a move decided by the try_improve method.
	 * \param ic considered move.
	 * \param solution current solution.
	 * \param statistics_moves statistics to be updated.
	 * \param undo flag which is true if the move has to be undone.
	 * \return a value to be added to the current objective function value to reflect the modification.
	**/
	double add_remove_user(const improved_move& ic, multi_array<int, 4>& solution, moves_statistics& statistics_moves, const bool undo);
};

/** \brief Data structure containing different information about which groups of users have been moved
 * to generate the solution. It is used to execute the improving phase. **/
struct coiote_solver::moves_statistics {
	/** \brief Number of users per each group still available at the end. **/
	multi_array<int, 3> users_available;

	/** \brief Array containing each move done to get the solution. **/
	vector_moves_type moves;
	/** \brief Array containing for each source cell the moves done to get the solution. **/
	std::vector<vector_moves_type> moves_from_i;
	/** \brief Array containing for each destination cell the moves done to get the solution. **/
	std::vector<vector_moves_type> moves_to_j;

	/** \brief Number of activities done in each destination cell. **/
	std::vector<int> done_in_j;

	/**
	 * \brief Constructor.
	 * \param n_cells number of cells.
	 * \param n_cust_types number of different customer types.
	 * \param n_time_steps number of different time periods.
	**/
	moves_statistics(const size_type& n_cells, const size_type& n_cust_types, const size_type& n_time_steps)
		: users_available({n_cells, n_cust_types, n_time_steps}),
			moves_from_i(n_cells), moves_to_j(n_cells),
			done_in_j(n_cells, 0) {}
};

/** \brief Data structure used to contain an improved move (i.e. a change in the solution
 * discovered by the try_improve function which chained with a series of others leads
 * to a smaller value of the objective function value. **/
struct coiote_solver::improved_move {
	three_index_type t_idx; /**< \brief Index of the modified cell (three dimensional) **/
	four_index_type f_idx; /**< \brief Index of the modified cell (four dimensional) **/
	int user_added; /**< \brief Number of added (or removed) users. **/
	int activities_added; /**< \brief Number of added (or removed) activities. **/
	double obj_gain; /**< \brief Gain (or loss) obtained through this move. **/

	/**
	 * \brief Constructor.
	 * \param i source cell.
	 * \param j destination cell.
	 * \param m customers type.
	 * \param t time period.
	 * \param user_added number of added (or removed) users.
	 * \param activities_added number of added (or removed) activities.
	 * \param obj_gain gain (or loss) obtained through this move.
	**/
	improved_move(const size_type& i, const size_type& j, const size_type& m, const size_type& t,
		const int user_added, const int activities_added, const double obj_gain)
		: t_idx({i,m,t}), f_idx({i,j,m,t}), user_added(user_added),
			activities_added(activities_added), obj_gain(obj_gain) {}
};

/** \brief Data structure used as a parameter for the function thread_body. **/
struct coiote_solver::th_parameter {
	multi_array<int, 4> solution; /**< \brief Best solution found so far. **/
	double obj_function; /**< \brief Value of the objective function relative to the best solution found so far. **/
	std::mt19937 rndgen; /**< \brief Random genarator unique for each thread_body execution. **/
	size_type iterations; /**< \brief Number of iterations done during the thread body execution. **/

	/** \brief Dimensions of the problem used to build three dimensional arrays. **/
	const three_index_type three_dimensions;
	/** \brief Dimensions of the problem used to build four dimensional arrays. **/
	const four_index_type four_dimensions;

	/**
	 * \brief Constructor.
	 *
	 * \param seed seed for the random generator.
	 * \param t_dim dimensions of the problem used to build three dimensional arrays.
	 * \param f_dim dimensions of the problem used to build four dimensional arrays.
	**/
	th_parameter(const unsigned seed, const three_index_type& t_dim, const four_index_type& f_dim)
		: solution(f_dim), obj_function(std::numeric_limits<double>::infinity()),
			rndgen(seed), iterations(0), three_dimensions(t_dim), four_dimensions(f_dim) {}
};

/** \brief Data structure used as a parameter for the function try_improve. **/
struct coiote_solver::ti_parameter {
	const unsigned it_level; /**< \brief Current iteration level. **/
	const four_index_type curr_idx; /**< \brief Cell considered by the current iteration. **/
	const int users_to_remove; /**< \brief Number of users to be removed. **/

	double obj_gain_so_far; /**< \brief Gain of the objective function value so far. **/
	std::vector<coiote_solver::improved_move> imp_moves; /**< \brief List of moves already done. **/
	std::vector<four_index_type> considered_cells; /**< \brief List of tabu cells. **/

	/**
	 * \brief Constructor (brand new parameter).
	 * \param idx cell to be considered.
	 * \param users_to_remove number of users to be removed.
	**/
	ti_parameter(const four_index_type& idx, const int users_to_remove)
		: it_level(0), curr_idx(idx), users_to_remove(users_to_remove),
			obj_gain_so_far(0), imp_moves(), considered_cells() {}

	/**
	 * \brief Constructor (given an existing parameter).
	 * \param current instance of this structure to be used as a starting point.
	 * \param idx cell to be considered.
	 * \param users_to_remove number of users to be removed.
	**/
	ti_parameter(const ti_parameter& current, const four_index_type& idx, const int users_to_remove)
		: it_level(current.it_level+1), curr_idx(idx), users_to_remove(users_to_remove),
			obj_gain_so_far(current.obj_gain_so_far), imp_moves(),
			considered_cells(current.considered_cells) {}

	/** \brief Resets the content of the structure, in order to be able to reiterate. **/
	void clear() {
		obj_gain_so_far = 0;
		imp_moves.clear();
		considered_cells.clear();
	}
};

/**
 * \brief Data structure that contains information about the usage of the groups of users.
 *
 * This class stores, for each group of users (source cell, type and time period),
 * a number which represents how much it has been important in the previous iterations,
 * with the purpose of preferring, when choosing between users with the same cost, the
 * ones which may be less useful to perform tasks in other cells.
**/
class coiote_solver::cells_usage {
public:
	/**
	 * \brief Constructor.
	 * \param th_dim dimensions of the problem used to build three dimensional arrays.
	 * \param users_available reference to the structure containing the total number of users available.
	**/
	cells_usage(const three_index_type& th_dim, const multi_array<int,3>& users_available)
		: usage(th_dim), users_available(users_available) {
			usage.reset();
	}

	/**
	 * \brief Adds the information into the statistic.
	 * \param idx index of the users added.
	 * \param nusers number of users added.
	**/
	inline void add(const three_index_type& idx, unsigned nusers) {
		usage[idx] += ((double)nusers/users_available[idx]);
	}

	/**
	 * \brief Compares two groups of users and returns which one may be the best to be chosen.
	 * \param new_idx users group which is available to replace the other one.
	 * \param old_idx previously chosen users group.
	 * \return a boolean parameter equal to true if it could be better to replace the previously
	 * chosen users group.
	**/
	inline bool should_replace(const three_index_type& new_idx, const three_index_type& old_idx) {
		return (usage[new_idx] < usage[old_idx]);
	}

private:
	multi_array<double, 3> usage; /**< \brief Data structure storing the usages. **/
	const multi_array<int, 3>& users_available; /**< \brief Total number of users available. **/
};

/**
 * \brief Comparator used to order the indexes according to not-increasing costs.
 *
 * In particular, this comparator is used when there is some exceed of activities
 * done and, for this reason, it is necessary to sort the users moved to that
 * destination cell in order to remove the most expensive one (if possible).
**/
class coiote_solver::cmp_costs_desc {
public:
	/**
	 * \brief Constructor.
	 * \param costs reference to the structure containing the costs of each move,
	 * which is saved to be able to perform the comparison.
	**/
	cmp_costs_desc(const multi_array<double, 4>& costs) : costs(costs) {}

	/**
	 * \brief Returns whether its first argument compares less than the second
	 * according to the not-increasing cost order (i.e. if cost[lhs] > cost[rhs]).
	 * \param lhs first argument.
	 * \param rhs second argument.
	 * \return comparison result.
	**/
	bool operator()(const four_index_type& lhs, const four_index_type& rhs) const {
		return costs[lhs] > costs[rhs];
	}
private:
	/** \brief Reference to the structure containing the costs of each move. **/
	const multi_array<double, 4>& costs;
};

/**
 * \brief Comparator used to order the indexes according to not-decreasing costs.
 *
 * The ordering is done considering the costs to be sorted as reduced by a factor
 * which is the minimum between the number of activities the given users type can
 * do and a limit specified as a parameter of the constructor.
 * Such a limit and in particular the computation of different orders with
 * different limits is done to be able to consider the correct sequence also
 * when only few activities have to be performed.
**/
class coiote_solver::cmp_costs_asc {
public:
	/**
	 * \brief Constructor.
	 * \param costs reference to the structure containing the costs of each move,
	 * which is saved to be able to perform the comparison.
	 * \param act_per_user array containing the number of activities each type of
	 * users is able to perform.
	 * \param max_done maximum number of the activities each user type is allowed
	 * to do. This is an additional constraint not related to the characteristics of
	 * the users but used to compute different orders as explained in the description
	 * of this class.
	**/
	cmp_costs_asc(const multi_array<double, 4>& costs, const int* act_per_user, const int max_done)
		: costs(costs), act_per_user(act_per_user), max_done(max_done) {}

	/**
	 * \brief Returns whether its first argument compares less than the second
	 * according to the not-decreasing reduced cost order.
	 * \param lhs first argument.
	 * \param rhs second argument.
	 * \return comparison result.
	**/
	bool operator()(const four_index_type& lhs, const four_index_type& rhs) const {
		return ((costs[lhs]/std::min(act_per_user[lhs[four_index::m]], max_done))
				< (costs[rhs]/std::min(act_per_user[rhs[four_index::m]], max_done)));
	}
private:
	/** \brief Reference to the structure containing the costs of each move. **/
	const multi_array<double, 4>& costs;

	/** \brief Number of activities each type of users is able to perform. **/
	const int* act_per_user;

	/** \brief Maximum number of activities each type of users is allowed to perform. **/
	const int max_done;
};


#endif
