/**
 * @file	event.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#ifndef __MEV_EVENT_HPP__
#define __MEV_EVENT_HPP__

namespace mev {
	class loop;
	class event_source;

	enum event_type {
		EVENT_NONE = 0,
		EVENT_READ = 0x00000002,
		EVENT_WRITE = 0x00000004,
		EVENT_ONCE = 0x00010000
	};

	struct event_info {
		size_t transferred_size;
		void* platform_event_ctx;
		event_info() {
			transferred_size = 0;
			platform_event_ctx = NULL;
		}
		virtual ~event_info() {}
	};

	class event_handler {
	public:
		virtual int mev_event(loop *_loop, const safeobj<event_source> &evt_src, event_type evt_type, void *userctx, event_info *evt_info) = 0;
	};
}

#endif /* __MEV_EVENT_HPP__ */
