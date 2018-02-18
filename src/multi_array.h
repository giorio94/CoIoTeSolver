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


#ifndef MULTI_ARRAY_H
#define MULTI_ARRAY_H

#include <algorithm>
#include <array>
#include <vector>

/**
 * \brief This class provides an efficient and easy to use implementation of multi-dimensional arrays.
 *
 * In a similar way than arrays do, multi_array uses contiguous storage locations for its elements,
 * which means that they can also be accessed using offsets on regular pointers to its elements,
 * and just as efficiently as in arrays.
 *
 * Nonetheless, the access to a given element is simplified by the implementation of the standard
 * operator[] that, given an index of the correct type, is able to retrieve the desired element without
 * having to manage with complex and error-prone offset calculations.
 *
 * Be careful, because, in order to make both instantiation and access as fast as possible,
 * neither boundary check nor correctness controls are performed.
 *
 * \tparam T: Type of elements to be stored into the multi-dimensional array.
 * \tparam N: Number of dimension such container is caracterized by.
 **/
template <class T, size_t N>
class multi_array {

public:
	/** \brief value_type is defined as an alias of T, the type of elements stored into the container. **/
	typedef T value_type;
	/** \brief size_type is defined as an alias of size_t, an unsigned integral type. **/
	typedef size_t size_type;
	/** \brief container_type is defined as an alias of the type of the current class. **/
	typedef multi_array<value_type, N> container_type;

	/** \brief const_iterator is defined as an alias of const value_type*, a random access iterator to const value_type. **/
	typedef const value_type* const_iterator;
	/** \brief iterator is defined as an alias of value_type*, a random access iterator to value_type. **/
	typedef value_type* iterator;
	/** \brief index_type is defined as an alias of std::array<size_type, N>. It represents an index for the elements of the container. **/
	typedef std::array<size_type, N> index_type;

	/**
	 * \brief Constructor.
	 *
	 * Constructs a multi-dimensional array, with the number of elements for each dimension specified
	 * by the parameter. The elements inside the array are not initialized with any value.
	 *
	 * \param dimensions an array containing for each dimension the number of elements to be allocated.
	**/
	multi_array(const index_type& dimensions) {
		this->size = 1;
		for(size_type n = 0; n < N; n++)
			this->size *= dimensions[n];
		this->dimensions = dimensions;
		data = new value_type[this->size];
	}

	/**
	 * \brief Copy Constructor.
	 *
	 * Constructs a container with a copy of each of the elements in other, in the same order.
	 *
	 * \param other another container of the same type (with the same class template arguments T and N).
	**/
	multi_array(const container_type& other) {
		this->size = other.size;
		this->dimensions = other.dimensions;
		this->data = new value_type[this->size];
		std::copy(other.begin(), other.end(), this->begin());
	}

	/**
	 * \brief Destructor
	**/
	~multi_array() {
		delete[](this->data);
	}

	/**
	 * \brief Assigns new contents to the container, replacing its current contents, and modifying its size accordingly.
	 *
	 * The object that calls this member class function becomes a copy of the object passed by reference.
	 *
	 * The objects must be of same type and must have the same number of dimensions but can have different
	 * number of elements for each dimension. If the two containers are caracterized by the same number of
	 * elements per dimension, the elements are simply copied, while in the opposite case the old container
	 * is destroyed and a brand new one is then allocated and the elements are copied.
	 *
	 * \param other another container of the same type (with the same class template arguments T and N).
	**/
	container_type& operator=(const container_type& other) {
		if(this != &other) {
			if(dimensions != other.dimensions) {
				this->size = other.size;
				this->dimensions = other.dimensions;

				delete[](this->data);
				data = new value_type[this->size];
			}

			std::copy(other.begin(), other.end(), this->begin());
		}
		return *this;
	};

	/**
	 * \brief Resets the multi-dimensional array.
	 *
	 * In particular, the array is initialized with copies returned by the default constructor of value_type
	 * (e.g. zero for numeric types).
	**/
	inline void reset() { std::fill(this->begin(), this->end(), value_type()); }

	/**
	 * \brief Returns a reference to the element at position specified by index.
	 * \param index the index referred to the desired element.
	 * \return reference to the desired element.
	**/
	inline value_type& operator[](const index_type& index) { return *get(index); }

	/**
	 * \brief Returns a constant reference to the element at position specified by index.
	 * \param index the index referred to the desired element.
	 * \return constant reference to the desired element.
	***/
	inline const value_type& operator[](const index_type& index) const { return *get(index); }

	/**
	 * \brief Returns an iterator pointing to the first element of the container object.
	 * \return iterator pointing to the first element.
	**/
	inline iterator begin() { return data; }

	/**
	 * \brief Returns a const_iterator pointing to the first element of the container object.
	 * \return const_iterator pointing to the first element.
	**/
	inline const_iterator begin() const { return data; }

	/**
	 * \brief Returns an iterator pointing to the past-the-end element of the container object.
	 *
	 * The past-the-end element is the theoretical element that would follow the last element in the vector.
	 * It does not point to any element, and thus shall not be dereferenced.
	 *
	 * \return const_iterator pointing to the past-the-end element.
	**/
	inline iterator end() { return data+size; }

	/**
	 * \brief Returns a const_iterator pointing to the past-the-end element of the container object.
	 *
	 * The past-the-end element is the theoretical element that would follow the last element in the vector.
	 * It does not point to any element, and thus shall not be dereferenced.
	 *
	 * \return const_iterator pointing to the past-the-end element.
	**/
	inline const_iterator end() const { return data+size; }

	/**
	 * \brief Returns an iterator pointing to the element specified by the index.
	 * \param index the index referred to the desired element.
	 * \return iterator pointing to the requested element.
	**/
	inline iterator get_iterator(const index_type& index) { return get(index); }

	/**
	 * \brief Returns a constant iterator pointing to the element specified by the index.
	 * \param index the index referred to the desired element.
	 * \return const_iterator pointing to the requested element.
	**/
	inline const_iterator get_iterator(const index_type& index) const { return get(index); }

private:
	/** \brief Pointer to the first element of the underlying dinamically allocated array. **/
	iterator data;
	/** \brief Number of elements of the underlying dinamically allocated array. **/
	size_type size;
	/** \brief Number of elements for each dimension of the multi-dimensional array. **/
	index_type dimensions;

	/**
	 * \brief Returns an iterator pointing to the element specified by the index.
	 * It is used by the other public class member functions.
	 *
	 * \param index the index referred to the desired element.
	 * \return iterator pointing to the requested element.
	 *
	 * \see begin()
	 * \see end()
	 * \see get_iterator()
	**/
	iterator get(const index_type& index) const {
		size_type idx = index[0];
		for(size_type n = 1; n < N; n++)
			idx = idx*dimensions[n] + index[n];
		return (data+idx);
	}
};

#endif
