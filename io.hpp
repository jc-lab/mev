/**
 * @file	io.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @class	mev::io
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#ifndef __MEV_IO_HPP__
#define __MEV_IO_HPP__

#include "safeobj.hpp"
#include "event_source.hpp"

namespace mev {
	class loop;

	class io : public event_source {
	protected:
		friend class loop;

	public:
		// OS dependent
		// int init(fd/handle, ...)
	};
}

#endif /* __MEV_IO_HPP__ */
