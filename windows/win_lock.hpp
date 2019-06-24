/**
 * @file	win_lock.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @class	mev::windows::win_lock
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#ifndef __MEV_WINDOWS_WIN_LOCK_HPP__
#define __MEV_WINDOWS_WIN_LOCK_HPP__

#include "../mev_common.hpp"

#if defined(MEV_WINDOWS) && MEV_WINDOWS

#include "../lock.hpp"
#include <Windows.h>

namespace mev {
	class io;
	namespace windows {
		class win_lock : public lock {
		private:
			win_lock* self_;
			SRWLOCK lock_;
			bool inited_;
			
			win_lock();
			win_lock(win_lock& o) {}

			int startup();

		public:
			~win_lock();

			int read_trylock() override;
			int read_lock() override;
			int read_unlock() override;
			int write_trylock() override;
			int write_lock() override;
			int write_unlock() override;

			static safeobj<lock> new_lock();
		};

		class win_mutex : public mutex {
		private:
			win_mutex* self_;
			CRITICAL_SECTION cs_;
			bool inited_;

			win_mutex();
			win_mutex(win_mutex& o) {}

			int startup();

		public:
			~win_mutex();

			int trylock() override;
			int lock() override;
			int unlock() override;
			void *owner_tid() override;
			
			static safeobj<mutex> new_mutex();
		};
	}
}

#endif /* MEV_WINDOWS */

#endif /* __MEV_WINDOWS_WIN_LOCK_HPP__ */
