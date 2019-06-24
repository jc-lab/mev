/**
 * @file	mev_common.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#ifndef __MEV_COMMON_HPP__
#define __MEV_COMMON_HPP__

#if defined(__linux) && __linux
#define MEV_LINUX 1
#define MEV_USE_EPOLL 1
#elif defined(_WIN32) && defined(_MSC_VER)
#define MEV_WINDOWS 1
#define MEV_USE_IOCP 1
#endif

#endif /* __MEV_COMMON_HPP__ */
