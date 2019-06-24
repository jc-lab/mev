/**
 * @file	look.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @class	mev::look
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#include "safeobj.hpp"

namespace mev {
	/*
	 * Read/Write Lock
	 * MUST DO NOT recursive lock.
	 */
	class lock {
	public:
		enum lock_type {
			READLOCK,
			WRITELOCK,
		};

		virtual int read_trylock() = 0;
		virtual int read_lock() = 0;
		virtual int read_unlock() = 0;
		virtual int write_trylock() = 0;
		virtual int write_lock() = 0;
		virtual int write_unlock() = 0;
	};

	/*
	 * Can recursive lock.
	 */
	class mutex {
	public:
		virtual int trylock() = 0;
		virtual int lock() = 0;
		virtual int unlock() = 0;
		virtual void* owner_tid() = 0;
	};

	class auto_lock {
	private:
		lock& o_;
		int state_;

	public:
		auto_lock(lock& o, lock::lock_type type) : o_(o), state_(0) {
			if (type == lock::READLOCK) {
				o_.read_lock();
				state_ = 1;
			}else if (type == lock::WRITELOCK) {
				o_.write_lock();
				state_ = 2;
			}
		}
		bool check() {
			return state_ > 0;
		}
		bool unlock() {
			switch (state_)
			{
			case 1:
				o_.read_unlock();
				state_ = 0;
				return true;
			case 2:
				o_.write_unlock();
				state_ = 0;
				return true;
			}
			return false;
		}
		~auto_lock() {
			unlock();
		}
	};

	safeobj<lock> default_lock();
	safeobj<mutex> default_mutex();
}
