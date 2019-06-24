/**
 * @file	iocp_loop.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @class	mev::windows::iocp_loop
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#ifndef __MEV_WINDOWS_IOCP_LOOP_HPP__
#define __MEV_WINDOWS_IOCP_LOOP_HPP__

#include "../mev_common.hpp"

#if defined(MEV_WINDOWS) && MEV_WINDOWS

#include "../loop.hpp"
#include "../lock.hpp"
#include "../event.hpp"
#include <Windows.h>

#include <list>
#include <map>
#include <set>

namespace mev {
	class io;

	namespace windows {
		enum evtsrctype;

		class iocp_loop : public loop {
		private:
			struct evtsrc;
			template<class T>
			struct evtsrc_impl;
			struct evtctx;
			template<class T>
			struct evtctx_impl;
			template<class EVENTSOURCE>
			static void evtctx_impl_deleter(evtctx_impl<EVENTSOURCE>* ptr);

			iocp_loop* self_;
			HANDLE iocp_handle_;
			HANDLE timer_queue_handle_;

			safeobj<lock> evtctx_lock_;
			std::map< event_source*, safeobj<evtsrc> > evtsrc_map_;
			std::set< safeobj<evtctx>, safeobj<evtctx>::comp > evtctx_set_;

		private:
			iocp_loop();
			iocp_loop(iocp_loop& o) {}

			int init(int num_of_concurrent_threads);
			static VOID CALLBACK timer_cb(_In_ PVOID lpParameter, _In_ BOOLEAN timerOrWaitFired);

			template<class EVENTSOURCE>
			void cleanup_event(evtctx_impl<EVENTSOURCE>* ctx);
			template<>
			void cleanup_event<timer>(evtctx_impl<timer>* ctx);
			template<>
			void cleanup_event<io>(evtctx_impl<io>* ctx);

			template<class EVENTSOURCE>
			int add_event_templ(const safeobj<event_source>& src, evtsrctype src_type, event_type type, event_handler* handler, void* userctx, void** platform_event_ctx);
			template<class EVENTSOURCE>
			int add_event_impl(const safeobj< evtsrc_impl<EVENTSOURCE> >& srcobj, const safeobj< evtctx_impl<EVENTSOURCE> > & evtctxobj, event_type type);

			static void* evtctx_from_ov(LPOVERLAPPED pov)
			{
				uint32_t* psize = ((uint32_t*)(((unsigned char*)pov) - 4));
				return ((void*)(((unsigned char*)pov) - (*psize)));
			}

		public:
			virtual ~iocp_loop();

			int do_loop(int timeout_ms) override;
			int add_event(const safeobj<event_source>& src, event_type type, event_handler* handler, void* userctx, void** platform_event_ctx) override;
			int del_event(event_source* src, event_type type) override;
			int close_event(event_source* src) override;

			void close();

			static safeobj<loop> new_loop(int num_of_concurrent_threads = 2);
		};
	}
}

#endif /* MEV_WINDOWS */

#endif /* __MEV_WINDOWS_IOCP_LOOP_HPP__ */
