/**
 * @file	safeobj.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @class	mev::safeobj
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#ifndef __MEV_SAFEOBJ_H__
#define __MEV_SAFEOBJ_H__

#include <assert.h>

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

namespace mev {
	namespace internal {
		class safeobj_store_base {
		public:
			int use_;
			int result_;
			safeobj_store_base() {
				use_ = 0;
			}
			virtual ~safeobj_store_base() {
			}
			virtual void* ptr() = 0;
			virtual void release() = 0;

			virtual int inc_ref() {
				return ++use_;
			}
			virtual int dec_ref() {
				int after = --use_;
				if (after == 0)
					release();
				return after;
			}
		};

		template <class U, typename DELETER>
		class safeobj_store : public safeobj_store_base {
		public:
			U* obj_;
			DELETER deleter_;

			safeobj_store(U* obj, int result, DELETER deleter) {
				obj_ = obj;
				result_ = result;
				deleter_ = deleter;
			}
			virtual ~safeobj_store() {
				if (obj_) {
					if (deleter_)
						deleter_(obj_);
					else
						delete obj_;
					obj_ = NULL;
				}
			}
			void* ptr() override { return obj_; }
			void release() override
			{
				delete this;
			}
		};

		class safeobj_base {
		protected:
			safeobj_store_base* store_;

			safeobj_base() {
				store_ = NULL;
			}

			virtual ~safeobj_base() {
				if(store_)
					store_->dec_ref();
			}

			void copyfrom(safeobj_base* src) {
				if (store_ != src->store_)
				{
					if (store_)
						store_->dec_ref();
					store_ = src->store_;
					store_->inc_ref();
				}
			}
		};
	}

	/**
	 * unique_ptr with result code.
	 */
	template <class T>
	class safeobj : public internal::safeobj_base {
	private:
		T* obj_;

	public:
		safeobj() {
			store_ = NULL;
			obj_ = NULL;
		}
		explicit safeobj(T* obj, int result = 0) {
			store_ = new internal::safeobj_store<T, void(*)(void*)>(obj, result, 0);
			obj_ = obj;
			store_->inc_ref();
		}
		template<typename DELETER>
		explicit safeobj(T* obj, int result, DELETER deleter) {
			store_ = new internal::safeobj_store<T, DELETER>(obj, result, deleter);
			obj_ = obj;
			store_->inc_ref();
		}
		/*safeobj(safeobj& src) {
			copyfrom(&src);
		}*/
		safeobj(const safeobj& src) {
			store_ = NULL; obj_ = NULL;
			copyfrom((safeobj_base*)& src);
			obj_ = src.get();
		}
		safeobj(safeobj&& src) {
			store_ = NULL; obj_ = NULL;
			copyfrom(&src);
			obj_ = src.get();
		}
		/*
		safeobj& operator=(safeobj& src) {
			release();
			copyfrom(&src);
			return *this;
		}*/
		safeobj& operator=(const safeobj& src) {
			obj_ = NULL;
			copyfrom((safeobj_base *)&src);
			obj_ = src.get();
			return *this;
		}
		template<class U>
		safeobj(const safeobj<U>& src) {
			store_ = NULL; obj_ = NULL;
			copyfrom((safeobj_base*)&src);
			obj_ = dynamic_cast<T*>(src.root<U>());
		}
		template<class U>
		safeobj(safeobj<U>&& src) {
			store_ = NULL; obj_ = NULL;
			copyfrom(&src);
			obj_ = dynamic_cast<T*>(src.root<U>());
		}
		/*
		template<class U>
		safeobj& operator=(safeobj<U>& src) {
			obj_ = NULL;
			copyfrom(&src);
			return *this;
		}
		*/
		template<class U>
		safeobj& operator=(const safeobj<U>& src) {
			obj_ = NULL;
			copyfrom((safeobj_base*)& src);
			obj_ = dynamic_cast<T*>(src.get());
			return *this;
		}
		void set(T* obj, int result = 0) {
			release();
			store_ = new internal::safeobj_store<T, void(*)(void*)>(obj, result, 0);
			obj_ = obj;
			store_->inc_ref();
		}

		void release() {
			obj_ = NULL;
			if (store_)
				store_->dec_ref();
			store_ = NULL;
		}

		T* operator->() const {
			return obj_;
		}

		T* get() const {
			return obj_;
		}

		template<class U>
		U* root() const {
			if (store_)
				return (U*)store_->ptr();
			return NULL;
		}

		operator T*() const {
			return obj_;
		}

		int result() const {
			if (store_)
				return store_->result_;
			return 0;
		}

		void set_result(int result) {
			if (store_)
				store_->result_ = result;
		}

		bool owner() const {
			return (store_ != NULL);
		}
		/*
		bool operator==(const safeobj& tgt) const {
			return this->obj_ == tgt.ob;
		}
		bool operator==(const T* tgt) const {
			if (store_)
				return (store_->ptr() == tgt);
			else
				return (tgt == NULL);
		}
		*/

		bool operator<(const safeobj& tgt) const {
			if (store_ && tgt.store_)
				return store_->ptr() < tgt.store_->ptr();
			return store_ < tgt.store_;
		}

		bool operator<(const T *tgt) const {
			if (store_)
				return store_->ptr() < tgt;
			return store_ < tgt;
		}

		class comp {
		public:
			bool operator()(const safeobj& left, const safeobj& right) const {
				return left.root<void>() < right.root<void>();
			}
			bool operator()(const void *left, const safeobj& right) const {
				return left < right.root<void>();
			}
			bool operator()(const safeobj& left, const void*right) const {
				return left.root<void>() < right;
			}
		};
	};
}

#endif /* __MEV_SAFEOBJ_H__ */
