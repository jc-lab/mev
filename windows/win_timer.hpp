/**
 * @file	win_timer.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @class	mev::windows::win_timer
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#ifndef __MEV_WINDOWS_WIN_TIMER_HPP__
#define __MEV_WINDOWS_WIN_TIMER_HPP__

#include "../mev_common.hpp"

#if defined(MEV_WINDOWS) && MEV_WINDOWS
#include "../timer.hpp"
#include <Windows.h>

namespace mev {
	class io;
	namespace windows {
		class win_timer : public timer {
		private:
			win_timer* self_;

			int after_ms_;
			int period_ms_;

			void* userctx_;

			win_timer();
			win_timer(win_timer& o) {}

			int startup(int after_ms, int period_ms);

		public:
			~win_timer();
			
			void* io_handle() const override { return 0; };

			int get_after_ms() const override;
			int get_period_ms() const override;

			void event_closed(loop* _loop) override;
			int mev_event(const safeobj<event_source>& evt_src, event_handler* user_handler, loop* _loop, event_type evt_type, void* userctx, event_info* evt_info) override;

			static safeobj<timer> new_timer(int after_ms, int period_ms);
		};
	}
}

#endif

#endif /* __MEV_WINDOWS_WIN_TIMER_HPP__ */
