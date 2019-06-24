/**
 * @file	loop.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @class	mev::loop
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#ifndef __MEV_LOOP_HPP__
#define __MEV_LOOP_HPP__

#include "safeobj.hpp"
#include "event.hpp"

namespace mev {

	class io;
	class timer;
	class timer_handler;
	class event_handler;
	class loop {
	public:
		/**
		 * @return
		 *  == 0 : success
		 *  != 0 : error code
		 */
		virtual int do_loop(int timeout_ms = -1) = 0;

		/**
		 * Add event
		 * @param io : io pointer
		 * @param type : event_type
		 * @param event_ctx : user's event context
		 * @param auto_delete : Closes io when all events have been deleted.
		 */
		virtual int add_event(const safeobj<event_source>& src, event_type type, event_handler *handler, void* userctx, void** platform_event_ctx = NULL) = 0;

		/**
		 * Delete event
		 * If auto_delete was set and all events have been deleted, Closes io.
		 * @param io : io pointer
		 * @param type : event_type
		 */
		virtual int del_event(event_source*src, event_type type) = 0;

		/**
		 * delete all events.
		 */
		virtual int close_event(event_source* src) = 0;
	};

	/**
	 * Create default system loop.
	 */
	safeobj<loop> default_loop();
}

#endif /* __MEV_LOOP_HPP__ */
