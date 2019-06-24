/**
 * @file	handle.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @class	mev::handle
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#ifndef __MEV_HANDLE_HPP__
#define __MEV_HANDLE_HPP__

#include "safeobj.hpp"

namespace mev {
	class handle_base;

	template<typename T>
	class handle_closer {
	public:
		virtual void handle_close(handle_base& base, T h) = 0;
	};

	class handle {
	protected:
		template<typename T>
		struct close_base {
			virtual void operator()(handle_base &base, T h) = 0;
		};

		handle() { }
	private:
		handle(const handle& o) { assert(false); }
	public:
		virtual void* get_raw() const = 0;
	};
	
	template<typename T>
	class handle_impl : public handle {
	private:
		T handle_;
		int use_;
		close_base<T>* close_;

	private:
		template<typename CLOSEF>
		struct close_impl : public close_base<T>
		{
			CLOSEF func_;

			close_impl(CLOSEF func)
			{
				func_ = func;
			}

			void operator()(handle_base& base, T h) override
			{
				func_(h);
			}
		};

		template<>
		struct close_impl<handle_closer<T>*> : public close_base<T>
		{
			handle_closer<T>* closer_;

			close_impl(handle_closer<T>* closer)
			{
				closer_ = closer;
			}

			void operator()(handle_base& base, T h) override
			{
				closer_->handle_close(base, h);
			}
		};

		handle_impl() {
			handle_ = NULL;
			use_ = 0;
			close_ = NULL;
		}

		template<typename CLOSEF>
		handle_impl(CLOSEF closefunc) {
			handle_ = NULL;
			use_ = 0;
			close_ = new close_impl<CLOSEF>(closefunc);
		}

		void release() {
			if (close_ && handle_)
			{
				(*close_)(*this, handle_);
			}
			handle_ = NULL;
			use_ = 0;
		}

	public:
		~handle_impl() {
			release();
			if (close_) {
				delete close_;
			}
		}

		void set(T handle) {
			release();
			handle_ = handle;
			use_ = 0;
		}

		template<typename CLOSEF>
		void set(T handle, CLOSEF closefunc = NULL) {
			release();
			handle_ = handle;
			use_ = 0;
			if (close_) {
				delete close_;
				close_ = NULL;
			}
			if (closefunc)
				close_ = new close_impl<CLOSEF>(closefunc);
		}

		void* get_raw() const override {
			return (void*)handle_;
		}

		T get() const {
			return handle_;
		}

		void add_ref() {
			use_++;
		}

		void del_ref() {
			use_--;
			if (use_ == 0)
			{
				release();
			}
		}

		template<typename CLOSEF>
		static safeobj< handle_impl<T> > new_handle(CLOSEF closefunc) {
			handle_impl<T> *obj = new handle_impl<T>(closefunc);
			obj->self_ = obj;
			return obj;
		}

		template<typename CLOSEF>
		static safeobj< handle_impl<T> > new_handle(CLOSEF closefunc, T h) {
			handle_impl<T>* obj = new handle_impl<T>();
			obj->self_ = obj;
			obj->set(h, closefunc);
			return obj;
		}
	};
	
	template<typename T, typename CLOSEF>
	safeobj< handle_impl<T> > new_handle(CLOSEF closefunc, T h)
	{
		return handle_impl<T>::new_handle(closefunc, h);
	}

	template<typename T, typename CLOSEF>
	safeobj< handle_impl<T> > new_handle(CLOSEF closefunc)
	{
		return handle_impl<T>::new_handle(closefunc);
	}
	
}

#endif /* __MEV_IO_HPP__ */
