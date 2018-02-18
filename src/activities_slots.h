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


#ifndef ACTIVITIES_SLOTS_H
#define ACTIVITIES_SLOTS_H

#include <algorithm>

/**
 * \brief Class implementing slots for the activities to be executed.
 *
 * The main purpose of this auxiliary class, actually used only in the case
 * of instances with very limited amount of users in excess, is to provide some slots
 * (by introducing some constraints) in order to try to satisfy the requests
 * without wasting any available activity.
 *
 * In other words, an array with a number of elements equal to the maximum number
 * of activities to be done is created and, for each type of users, it is stored
 * whether selecting an user of that type, in case the given number (from zero to
 * the maximum) of activities have still to be executed, will by sure, in the end,
 * lead to some wasting or if it may be possible to slot in correctly the different
 * users.
**/
class activities_slots {
public:
	/** \brief size_type is defined as an alias of size_t, an unsigned integral type. **/
	typedef size_t size_type;

	/**
	 * \brief Constructor.
	 *
	 * It builds up the slots starting from different characteristics of the current instance.
	 *
	 * \param max_activities maximum number of activities to be done.
	 * \param n_cust_types number of different types of users.
	 * \param act_per_user array storing for each type of users the number of activities he is able to do.
	**/
	activities_slots(const int max_activities, const size_type& n_cust_types, const int* const act_per_user)
		: data(max_activities+1, std::vector<bool>(n_cust_types+1, false)), gen_idx(n_cust_types) {

		int idx;
		std::fill(data[0].begin(), data[0].end(), true);
		for(int a = 0; a <= max_activities; a++) {
			for(size_type m = 0; m < n_cust_types; m++) {
				if((idx = a-act_per_user[m]) >= 0) {
					data[a][m] = data[idx][gen_idx];
					data[a][gen_idx] = data[a][gen_idx] || data[a][m];
				}
			}
		}
	}

	/**
	 * \brief Returns true if the current demand cannot be correctly satisfied by any user type.
	 *
	 * \param demand remaining demand still to be satisfied.
	 * \return boolean value.
	**/
	inline bool should_skip(const int demand) {
		return !data[demand][gen_idx];
	}

	/**
	 * \brief Returns true if the current demand could eventually be correctly satisfied
	 * by using the specified user type.
	 *
	 * \param demand remaining demand still to be satisfied.
	 * \param m index of the selected user type.
	 * \return boolean value.
	**/
	inline bool can_be_selected(const int demand, const size_type& m) {
		return demand >= 0 && data[demand][m];
	}

private:
	/** \brief Data stucture storing the slots information. **/
	std::vector<std::vector<bool>> data;
	/** \brief Number of different user types **/
	const size_type gen_idx;
};

#endif
