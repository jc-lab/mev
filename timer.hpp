/**
 * @file	timer.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @class	mev::timer
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#ifndef __MEV_TIMER_HPP__
#define __MEV_TIMER_HPP__

#include "safeobj.hpp"
#include "event_source.hpp"

namespace mev {
	class timer : public event_source {
	public:
		virtual int get_after_ms() const = 0;
		virtual int get_period_ms() const = 0;
	};

	safeobj<timer> default_timer(int after_ms, int period_ms);
}

#endif /* __MEV_TIMER_HPP__ */
