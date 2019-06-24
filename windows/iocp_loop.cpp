/**
 * @file	iocp_loop.cpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @class	mev::windows::iocp_loop
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#include "iocp_loop.hpp"

#if defined(MEV_WINDOWS) && MEV_WINDOWS
#include <Windows.h>
#include "../io.hpp"
#include "../timer.hpp"

#include <algorithm>

/*
 * IOCP Key : event_source pointer
 * IOCP Overlapped : OVERLAPPED pointer (Can find evtctx by OVERLAPPED pointer)
 */
namespace mev {
	safeobj<loop> default_loop()
	{
		return windows::iocp_loop::new_loop();
	}

	namespace windows {
		enum evtsrctype {
			EVT_UNKNOWN = 0,
			EVT_IO = 1,
			EVT_TIMER = 2,
		};

		struct iocp_loop::evtsrc {
			evtsrctype type;
			safeobj<mutex> mutex;
			std::list< safeobj<evtctx> > events;

			evtsrc() {
				mutex = default_mutex();
			}
			virtual ~evtsrc() {
			}

			virtual safeobj<event_source> get_obj() const = 0;
		};

		template<class T>
		struct iocp_loop::evtsrc_impl : public evtsrc {
			safeobj<T> obj;

			evtsrc_impl(loop* _loop, evtsrctype _type, safeobj<T> _obj)
			{
				this->type = _type;
				this->obj = _obj;
			}

			safeobj<event_source> get_obj() const override {
				return obj;
			}
		};

		struct iocp_loop::evtctx {
			evtsrctype type;
			iocp_loop* loop;
			event_type evt_type;
			event_source* srcid;
			LPOVERLAPPED pov;
			event_handler* evt_handler;
			void* evt_userptr;

			evtctx() { }
			virtual ~evtctx() {}
		};

		template<>
		struct iocp_loop::evtctx_impl<timer> : public iocp_loop::evtctx {
			safeobj<timer> srcobj;
			HANDLE handle;
			evtctx_impl(iocp_loop *loop, event_source* srcid, safeobj<timer> srcobj, event_type evt_type, event_handler* evt_handler, void * evt_userptr)
			{
				this->type = EVT_TIMER;
				this->loop = loop;
				this->srcid = srcid;
				this->srcobj = srcobj;
				this->handle = NULL;

				this->evt_type = evt_type;
				this->evt_handler = evt_handler;
				this->evt_userptr = evt_userptr;
			}
		};

		template<>
		struct iocp_loop::evtctx_impl<io> : public iocp_loop::evtctx {
			safeobj<io> srcobj;
			evtctx_impl(iocp_loop* loop, event_source* srcid, const safeobj<io>& srcobj, event_type evt_type, event_handler* evt_handler, void* evt_userptr)
			{
				this->type = EVT_IO;
				this->loop = loop;
				this->srcid = srcid;
				this->srcobj = srcobj;
				this->evt_type = evt_type;
				this->evt_handler = evt_handler;
				this->evt_userptr = evt_userptr;
			}
		};
		safeobj<loop> iocp_loop::new_loop(int num_of_concurrent_threads)
		{
			safeobj<iocp_loop> loop(new iocp_loop());
			loop->self_ = loop.get();
			loop.set_result(loop->init(num_of_concurrent_threads));
			if (loop.result() != 0)
				loop.release();
			return loop;
		}

		iocp_loop::iocp_loop()
		{
			self_ = NULL;
			iocp_handle_ = NULL;
			timer_queue_handle_ = NULL;
		}

		iocp_loop::~iocp_loop()
		{
			close();
		}

		int iocp_loop::init(int num_of_concurrent_threads)
		{
			DWORD dwError;

			evtctx_lock_ = default_lock();

			iocp_handle_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, num_of_concurrent_threads);
			if (!iocp_handle_ || iocp_handle_ == INVALID_HANDLE_VALUE)
			{
				dwError = ::GetLastError();
				return dwError;
			}
			timer_queue_handle_ = ::CreateTimerQueue();
			if (!timer_queue_handle_ || timer_queue_handle_ == INVALID_HANDLE_VALUE)
			{
				dwError = ::GetLastError();
				return dwError;
			}
			return 0;
		}

		int iocp_loop::do_loop(int timeout_ms)
		{
			DWORD dwTransferredBytes = 0;
			ULONG_PTR key = NULL; // event_source*
			LPOVERLAPPED pov = NULL;
			BOOL gqcs = ::GetQueuedCompletionStatus(iocp_handle_, &dwTransferredBytes, &key, &pov, (timeout_ms < 0) ? INFINITE : (DWORD)timeout_ms);
			if(key) {
				safeobj<evtctx> evtctxobj;
				safeobj<evtsrc> evtsrcobj;

				evtctx* evtctxbase = (evtctx*)evtctx_from_ov(pov);

				evtctx_lock_->read_lock();
				std::map< event_source*, safeobj<evtsrc> >::iterator iter_evtsrc = evtsrc_map_.find(evtctxbase->srcid);
				std::set< safeobj<evtctx>, safeobj<evtctx>::comp >::iterator iter_evtctx = std::find(evtctx_set_.begin(), evtctx_set_.end(), evtctxbase);
				if (iter_evtsrc != evtsrc_map_.end())
					evtsrcobj = iter_evtsrc->second;
				if (iter_evtctx != evtctx_set_.end())
					evtctxobj = *iter_evtctx;
				evtctx_lock_->read_unlock();

				printf("EVENT : %p %p %p :: %d\n", pov, evtsrcobj.get(), evtctxobj.get(), dwTransferredBytes);

				if (!evtsrcobj.get() || !evtctxobj.get())
				{
					printf("WARNING L1\n");
				}
				if (gqcs)
				{
					if (evtsrcobj.get() && evtctxobj.get()) {
						event_info info;
						info.transferred_size = dwTransferredBytes;
						info.platform_event_ctx = pov;
						evtsrcobj->mutex->lock();
						evtsrcobj->get_obj()->mev_event(evtsrcobj->get_obj(), evtctxbase->evt_handler, this, evtctxbase->evt_type, evtctxbase->evt_userptr, &info);
						if (evtctxbase->evt_type & EVENT_ONCE)
							del_event(evtsrcobj->get_obj().get(), evtctxbase->evt_type);
						evtsrcobj->mutex->unlock();
					}
				} else {
					// closed handle
					close_event(evtsrcobj->get_obj().get());
				}
			}else{
				DWORD dwError = ::GetLastError();
				if (dwError != WAIT_TIMEOUT)
					return dwError;
			}
			return 0;
		}

		void iocp_loop::close()
		{
			if (iocp_handle_ && (iocp_handle_ != INVALID_HANDLE_VALUE))
			{
				::CloseHandle(iocp_handle_);
				iocp_handle_ = NULL;
			}
			if (timer_queue_handle_ && (timer_queue_handle_ != INVALID_HANDLE_VALUE))
			{
				::DeleteTimerQueue(timer_queue_handle_);
				timer_queue_handle_ = NULL;
			}
		}

		template<class EVENTSOURCE>
		void iocp_loop::evtctx_impl_deleter(evtctx_impl<EVENTSOURCE>* ptr)
		{
			ptr->~evtctx_impl<EVENTSOURCE>();
			free(ptr);
		}

		template<class EVENTSOURCE>
		int iocp_loop::add_event_templ(const safeobj<event_source>& sosrc, evtsrctype src_type, event_type type, event_handler* handler, void* userctx, void** platform_event_ctx)
		{
			safeobj<EVENTSOURCE> src(sosrc);

			bool new_src = false;
			int rc = 0;
			HANDLE handle = NULL;
			safeobj< evtsrc_impl<EVENTSOURCE> > evtsrcobj;

			mev::auto_lock cs_1(*evtctx_lock_.get(), lock::WRITELOCK);

			std::map< event_source*, safeobj<evtsrc> >::iterator iter_src = evtsrc_map_.find(static_cast<event_source*>(sosrc.get()));
			if (iter_src == evtsrc_map_.end())
			{
				// New source
				evtsrcobj.set(new evtsrc_impl<EVENTSOURCE>(this, src_type, src));
				new_src = true;

				if (sosrc->io_handle())
				{
					handle = ::CreateIoCompletionPort((HANDLE)sosrc->io_handle(), iocp_handle_, (ULONG_PTR)static_cast<event_source*>(sosrc.get()), 0);
					if (!handle || handle == INVALID_HANDLE_VALUE)
					{
						DWORD dwErr = ::GetLastError();
						return (int)dwErr;
					}
				}
			} else {
				evtsrcobj = iter_src->second;
			}

			/*
			* evtctxbuf structure
			*      evtctx_impl<EVENTSOURCE>
			*      evtctx_size (= sizeof(evtctx_impl<EVENTSOURCE>) + 4 )
			*      OVERLAPPED <- *platform_event_ctx
			*/

			size_t evtctxsize = sizeof(OVERLAPPED) + sizeof(evtctx_impl<EVENTSOURCE>) + 4;
			unsigned char* evtctxbuf = (unsigned char*)malloc(evtctxsize);
			uint32_t* psizeindicator = (uint32_t*)(evtctxbuf + sizeof(evtctx_impl<EVENTSOURCE>));
			memset(evtctxbuf, 0, evtctxsize);
			*psizeindicator = sizeof(evtctx_impl<EVENTSOURCE>) + 4;
			evtctx_impl<EVENTSOURCE>* pevtctx = ((evtctx_impl<EVENTSOURCE>*)evtctxbuf);
			new (pevtctx) evtctx_impl<EVENTSOURCE>(this, static_cast<event_source*>(sosrc.get()), src, type, handler, userctx);
			safeobj< evtctx_impl<EVENTSOURCE> > evtctxobj((evtctx_impl<EVENTSOURCE>*)evtctxbuf, 0, evtctx_impl_deleter<EVENTSOURCE>);

			rc = add_event_impl<EVENTSOURCE>(evtsrcobj, evtctxobj, type);

			if (rc == 0) {
				pevtctx->pov = (LPOVERLAPPED)(&psizeindicator[1]);
				if (platform_event_ctx)
					* platform_event_ctx = pevtctx->pov;
				evtsrcobj->mutex->lock();
				evtsrcobj->events.push_back(evtctxobj);
				evtsrcobj->mutex->unlock();
				evtctx_set_.insert(evtctxobj);
				if (new_src)
					evtsrc_map_[static_cast<event_source*>(sosrc.get())] = evtsrcobj;
			}

			return rc;
		}

		template<>
		int iocp_loop::add_event_impl<io>(const safeobj< evtsrc_impl<io> >& srcobj, const safeobj< evtctx_impl<io> >& evtctxobj, event_type type)
		{
			return 0;
		}

		template<>
		int iocp_loop::add_event_impl<timer>(const safeobj< evtsrc_impl<timer> >& srcobj, const safeobj< evtctx_impl<timer> >& evtctxobj, event_type type)
		{
			HANDLE handle = NULL;
			if (!::CreateTimerQueueTimer(&handle, timer_queue_handle_, timer_cb, evtctxobj.get(), srcobj->obj->get_after_ms(), srcobj->obj->get_period_ms(), WT_EXECUTEDEFAULT))
			{
				DWORD dwErr = ::GetLastError();
				return (int)dwErr;
			}
			evtctxobj->handle = handle;
			return 0;
		}

		int iocp_loop::add_event(const safeobj<event_source>& src, event_type type, event_handler* handler, void* userctx, void** platform_event_ctx)
		{
			int rc = 0;
			evtsrctype src_type = EVT_UNKNOWN;
			io* evtsrc_io = dynamic_cast<io*>(src.get());
			timer* evtsrc_timer = dynamic_cast<timer*>(src.get());

			if (evtsrc_io)
				return add_event_templ<io>(src, EVT_IO, type, handler, userctx, platform_event_ctx);
			else if (evtsrc_timer)
				return add_event_templ<timer>(src, EVT_TIMER, type, handler, userctx, platform_event_ctx);

			return -1;
		}

		int iocp_loop::del_event(event_source* src, event_type type)
		{
			int rc = -1;

			safeobj<evtsrc> evtsrcobj; // will be set if should auto delete.
			safeobj<evtctx> evtctxobj;

			auto_lock cs_1(*evtctx_lock_.get(), lock::WRITELOCK);

			std::map< event_source*, safeobj<evtsrc> >::iterator iter_src = evtsrc_map_.find(src);
			if (iter_src != evtsrc_map_.end())
			{
				evtsrc* s = (evtsrc*)iter_src->second.get();
				s->mutex->lock();
				for (std::list< safeobj<evtctx> >::iterator iter_evtctx = s->events.begin(); iter_evtctx != s->events.end(); iter_evtctx++)
				{
					evtctx* ctx = iter_evtctx->get();
					if (ctx->evt_type & type)
					{
						rc = 0;
						evtctxobj = (*iter_evtctx);
						s->events.erase(iter_evtctx);
						evtctx_set_.erase(evtctxobj);
						break;
					}
				}
				s->mutex->unlock();
				if (s->events.size() == 0)
				{
					evtsrcobj = iter_src->second;
					evtsrcobj->get_obj()->event_closed(this);
					evtsrc_map_.erase(iter_src);
				}
			}

			if (evtsrcobj.get() != NULL)
			{
				switch (evtsrcobj->type & 0x0000FFFF)
				{
				case EVT_TIMER:
					cleanup_event<timer>(evtctxobj.root< evtctx_impl<timer> >());
					break;
				case EVT_IO:
					cleanup_event<io>(evtctxobj.root< evtctx_impl<io> >());
					break;
				}
			}

			return rc;
		}

		int iocp_loop::close_event(event_source *src)
		{
			int rc = -1;

			auto_lock cs_1(*evtctx_lock_.get(), lock::WRITELOCK);

			std::map< event_source*, safeobj<evtsrc> >::iterator iter_src = evtsrc_map_.find(src);
			if (iter_src != evtsrc_map_.end())
			{
				safeobj<evtsrc> evtsrcobj = iter_src->second;

				evtsrcobj->mutex->lock();
				for (std::list< safeobj<evtctx> >::iterator iter_evt = evtsrcobj->events.begin(); iter_evt != evtsrcobj->events.end(); )
				{
					switch (evtsrcobj->type & 0x0000FFFF)
					{
					case EVT_TIMER:
						cleanup_event<timer>(iter_evt->root< evtctx_impl<timer> >());
						break;
					case EVT_IO:
						cleanup_event<io>(iter_evt->root< evtctx_impl<io> >());
						break;
					}
					evtctx_set_.erase((*iter_evt));
					iter_evt = evtsrcobj->events.erase(iter_evt);
				}
				evtsrcobj->mutex->unlock();
				evtsrcobj->get_obj()->event_closed(this);
				evtsrc_map_.erase(iter_src);
			}

			return rc;
		}

		template<>
		void iocp_loop::cleanup_event<timer>(evtctx_impl<timer>* ctx)
		{
			HANDLE hCompletionEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			::DeleteTimerQueueTimer(timer_queue_handle_, ctx->handle, hCompletionEvent);
			::WaitForSingleObject(hCompletionEvent, INFINITE);
			::CloseHandle(hCompletionEvent);
		}

		template<>
		void iocp_loop::cleanup_event<io>(evtctx_impl<io>* ctx)
		{
		}

		VOID CALLBACK iocp_loop::timer_cb(_In_ PVOID lpParameter, _In_ BOOLEAN timerOrWaitFired)
		{
			evtctx_impl<timer>* ctx = (evtctx_impl<timer>*)lpParameter;
			::PostQueuedCompletionStatus(ctx->loop->iocp_handle_, 0, (ULONG_PTR)ctx->srcid, ctx->pov);
		}
	}
}

#endif
