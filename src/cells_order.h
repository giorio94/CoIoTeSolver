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


#ifndef CELLS_ORDER_H
#define CELLS_ORDER_H

#include <algorithm>
#include "multi_array.h"

/**
 * \brief Class implementing a simple way to order the cells of the cost matrix.
 *
 * In particular the implementation consists of a stripped-down version of a vector
 * class, providing contiguous storage locations to memorize indexes relative to
 * elements contained inside multi-dimensional arrays caracterized by four dimension.
 *
 * In order to make the whole process faster, the total capacity of this data structure
 * is fixed and must be set before inserting any element. For the same reason, neither
 * boundary check nor correctness controls are performed.
 *
 * The class also provides, after having ordered the indexes using the dedicated method,
 * a simple way to iterate through all the elements according to the cost order, by
 * automatically skipping those users no more available.
**/
class cells_order {
public:
	/** \brief size_type is defined as an alias of size_t. An unsigned integral type. **/
	typedef size_t size_type;

	/** \brief three_index_type represents the correct type to specify an element of a
	 * three dimensional array implemented through multi_array. **/
	typedef multi_array<int, 3>::index_type three_index_type;
	/** \brief four_index_type represents the correct type to specify an element of a
	 * four dimensional array implemented through multi_array. **/
	typedef multi_array<int, 4>::index_type four_index_type;

	/** \brief This accessory structure provides a mnemonic way to access the
	 * different indexes contained inside an element of four_index_type, without having
	 * to remember their position. **/
	struct four_index {
		static const size_type i = 0; /**< \brief Source cell. **/
		static const size_type j = 1; /**< \brief Destination cell. **/
		static const size_type m = 2; /**< \brief User type. **/
		static const size_type t = 3; /**< \brief Time period. **/
	};

	/** \brief value_type represents the type of data actually stored inside this container. **/
	typedef four_index_type value_type;
	/** \brief const_iterator is defined as an alias of const value_type*, a random access iterator to const value_type. **/
	typedef const value_type* const_iterator;
	/** \brief iterator is defined as an alias of value_type*, a random access iterator to value_type. **/
	typedef value_type* iterator;

	/**
	 * \brief Default constructor. Constructs an empty container, with no elements.
	**/
	cells_order() : _begin(nullptr), _end(nullptr), _capacity(nullptr) {}

	/**
	 * \brief Destructor.
	**/
	~cells_order() { delete[](_begin); }

	/**
	 * \brief Initializes the container with the given capacity, deleting the
	 * data previously stored (if any).
	 * \param capacity maximum number of elements that can be stored.
	**/
	void initialize(size_type capacity) {
		delete[](_begin);
		_begin = _end = new value_type[capacity];
		_capacity = _begin + capacity;
	}

	/**
	 * \brief Adds a new element at the end of the vector, after its current last element.
	 * \param item value to be copied to the new element.
	**/
	inline void push_back(const value_type& item) { *(_end++) = item; }

	/**
	 * \brief Sorts the data structure according to the comparator passed as parameter.
	 * \param comparator binary function that accepts two elements in the range as
	 * arguments, and returns a value convertible to bool. The value returned indicates
	 * whether the element passed as first argument is considered to go before the second
	 * in the specific strict weak ordering it defines. The function shall not modify any
	 * of its arguments. This can either be a function pointer or a function object.
	**/
	template <typename Comparator>
	inline void sort(Comparator comparator) { std::sort(_begin, _end, comparator); }

	/**
	  \brief Returns a const_iterator that points to the least expensive available user.
	  \param begin an const_iterator pointing to the first cell to be considered (for subsequent calls).
	  \param users_available a reference to the data structure containing the users still available.
	  \return const_iterator pointing to the least expensive element.
	**/
	inline const_iterator get_least_expensive(const_iterator begin,	const multi_array<int, 3>& users_available) const {
		while(begin != _end && users_available[convert_index(*begin)] <= 0)
			++begin;
		return begin;
	}

	/**
	 * \brief Returns a const_iterator pointing to the first element of the container object.
	 * \return const_iterator pointing to the first element.
	**/
	inline const_iterator begin() const { return _begin; }

	/**
	 * \brief Returns an iterator pointing to the past-the-end element of the container object.
	 *
	 * The past-the-end element is the theoretical element that would follow the last element in the vector.
	 * It does not point to any element, and thus shall not be dereferenced.
	 *
	 * \return const_iterator pointing to the past-the-end element.
	**/
	inline const_iterator end() const { return _end; }

private:
	/** \brief An iterator pointing to the first element in the container. **/
	iterator _begin;
	/** \brief An iterator pointing to the past-the-end inserted element in the container. **/
	iterator _end;
	/** \brief An iterator pointing to the past-the-end allocated element in the container. **/
	iterator _capacity;

	/**
	 * \brief Converts a four_index_type elmentent into a three_index_type one by removing the destination cell.
	 * \param idx a reference to the four_index_type to be converted.
	 * \return const three_index_type the converted index.
	**/
	const three_index_type convert_index(const four_index_type& idx) const {
		return { idx[four_index::i], idx[four_index::m], idx[four_index::t] };
	}
};

#endif
