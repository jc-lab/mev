/**
 * @file	win_timer.cpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @class	mev::windows::timer
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#include "win_lock.hpp"

#if defined(MEV_WINDOWS) && MEV_WINDOWS

#include <string>

namespace mev {
	safeobj<lock> default_lock()
	{
		return windows::win_lock::new_lock();
	}
	safeobj<mutex> default_mutex()
	{
		return windows::win_mutex::new_mutex();
	}

	namespace windows {
		safeobj<lock> win_lock::new_lock() {
			safeobj<win_lock> obj(new win_lock());
			obj->self_ = obj.get();
			obj.set_result(obj->startup());
			return obj;
		}

		win_lock::win_lock()
		{
			inited_ = false;
			memset(&lock_, 0, sizeof(lock_));
		}

		win_lock::~win_lock()
		{
		}

		int win_lock::startup()
		{
			InitializeSRWLock(&lock_);
			inited_ = true;
			return 0;
		}

		int win_lock::read_trylock()
		{
			return ::TryAcquireSRWLockShared(&lock_) ? 0 : -1;
		}

		int win_lock::read_lock()
		{
			::AcquireSRWLockShared(&lock_);
			return 0;
		}

		int win_lock::read_unlock()
		{
			::ReleaseSRWLockShared(&lock_);
			return 0;
		}

		int win_lock::write_trylock()
		{
			return ::TryAcquireSRWLockExclusive(&lock_) ? 0 : -1;
		}

		int win_lock::write_lock()
		{
			::AcquireSRWLockExclusive(&lock_);
			return 0;
		}

		int win_lock::write_unlock()
		{
			::ReleaseSRWLockExclusive(&lock_);
			return 0;
		}

	}

	namespace windows {
		safeobj<mutex> win_mutex::new_mutex() {
			safeobj<win_mutex> obj(new win_mutex());
			obj->self_ = obj.get();
			obj.set_result(obj->startup());
			return obj;
		}

		win_mutex::win_mutex()
		{
			inited_ = false;
			memset(&cs_, 0, sizeof(cs_));
		}

		win_mutex::~win_mutex()
		{
			if (inited_) {
				::DeleteCriticalSection(&cs_);
				inited_ = false;
			}
		}

		int win_mutex::startup()
		{
			InitializeCriticalSection(&cs_);
			inited_ = true;
			return 0;
		}

		int win_mutex::trylock()
		{
			return ::TryEnterCriticalSection(&cs_) ? 0 : -1;
		}

		int win_mutex::lock()
		{
			EnterCriticalSection(&cs_);
			return 0;
		}

		int win_mutex::unlock()
		{
			::LeaveCriticalSection(&cs_);
			return 0;
		}

		void* win_mutex::owner_tid()
		{
			return cs_.OwningThread;
		}

	}
}

#endif /* MEV_WINDOWS */
