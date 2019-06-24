/**
 * @file	win_timer.cpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @class	mev::windows::win_timer
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#include "win_timer.hpp"

#if defined(MEV_WINDOWS) && MEV_WINDOWS

#include <string>

namespace mev {
	safeobj<timer> default_timer(int after_ms, int period_ms)
	{
		return windows::win_timer::new_timer(after_ms, period_ms);
	}

	namespace windows {
		safeobj<timer> win_timer::new_timer(int after_ms, int period_ms) {
			safeobj<win_timer> obj(new win_timer());
			obj->self_ = obj.get();
			obj.set_result(obj->startup(after_ms, period_ms));
			return obj;
		}

		win_timer::win_timer()
		{
		}

		int win_timer::startup(int after_ms, int period_ms)
		{
			after_ms_ = after_ms;
			period_ms_ = period_ms;
			return 0;
		}

		win_timer::~win_timer()
		{
		}

		int win_timer::get_after_ms() const {
			return after_ms_;
		}
		int win_timer::get_period_ms() const {
			return period_ms_;
		}

		void win_timer::event_closed(loop* _loop) {}
		
		int win_timer::mev_event(const safeobj<event_source>& evt_src, event_handler* user_handler, loop* _loop, event_type evt_type, void* userctx, event_info* evt_info)
		{
			return user_handler->mev_event(_loop, evt_src, evt_type, userctx, NULL);
		}
	}
}

#endif /* MEV_WINDOWS */
