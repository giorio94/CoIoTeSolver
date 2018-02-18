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


#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <condition_variable>
#include <functional>
#include <thread>

/**
 * \brief This class provides an implementation of a stoppable timer.
 *
 * In particular, when an object of this class is contructed, it is specified
 * after how much time the timer expires. When such an event happens, a callback
 * function is executed and the timer is stopped.
 *
 * In case the timer is no more needed, it is possible to stop it using the
 * associated method or let the distructor to do the work.
**/
class timer {
private:
	/** \brief Conditional variable used to wait for the specified time in an efficient way. **/
	std::condition_variable cv;
	/** \brief Lock to be acquired before using the conditional variable. **/
	std::mutex cv_mutex;

	/** \brief Boolean variable specifying if the time expired. **/
	bool end;
	/** \brief Boolean variable specifying if the timer is stopped. **/
	bool stopped;

	/** \brief Thread used to implement the timer. **/
	std::thread th_timer;

	/**
	 * \brief Function implementing the timer
	 *
	 * This function is executed in its own thread and is entitled to wait for
	 * the given time and at the end to execute the callback function.
	 *
	 * \param ms duration of the timer (in milliseconds).
	 * \param callback function executed when the timer expires or is stopped.
	**/
	void timer_function(unsigned long ms, std::function<void(void)> callback) {
		std::unique_lock<std::mutex> lock(cv_mutex);
		cv.wait_for(lock, std::chrono::milliseconds(ms), [this](){return end;});
		end = true;
		callback();
	}

public:

	/**
	 * \brief Constructor.
	 *
	 * The constructor creates a timer object with the specified duration and
	 * immediately fires it. After the given time expires, or if the timer is
	 * stopped, the callback function is executed.
	 *
	 * \param milliseconds duration of the timer (in milliseconds).
	 * \param callback function executed when the timer expires or is stopped.
	**/
	timer(unsigned long milliseconds, std::function<void(void)> callback) :
		end(false), stopped(false), th_timer(&timer::timer_function, this, milliseconds, callback) {}

	/**
	 * \brief Destructor.
	**/
	~timer() {
		stop();
	}

	/**
	 * \brief Stops the timer if still running.
	**/
	void stop() {
		if(!stopped) {
			{
				std::lock_guard<std::mutex> lock(cv_mutex);
				end = true;
			}
			cv.notify_all();
			th_timer.join();
			stopped = true;
		}
	}
};

#endif
