/**
 * @file	event_source.hpp
 * @class   mev::event_source
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#ifndef __MEV_EVENT_SOURCE_HPP__
#define __MEV_EVENT_SOURCE_HPP__

#include "mev_common.hpp"
#if defined(MEV_WINDOWS) && MEV_WINDOWS
#include <windows.h>
#endif
#include "event.hpp"

namespace mev {
#if defined(MEV_WINDOWS) && MEV_WINDOWS
	typedef HANDLE handle_t;
#elif defined(MEV_LINUX) && MEV_LINUX
	typedef int handle_t;
#endif

	class event_source {
	public:
		virtual ~event_source() {}

		virtual handle_t io_handle() const = 0;

		virtual void event_closed(loop* _loop) = 0;

		virtual int mev_event(const safeobj<event_source>& evt_src, event_handler* user_handler, loop* _loop, event_type evt_type, void* userctx, event_info* evt_info) = 0;
	};
}

#endif /* __MEV_EVENT_SOURCE_HPP__ */
