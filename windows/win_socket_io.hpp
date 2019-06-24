/**
 * @file	win_socket_io.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#ifndef __MEV_WINDOWS_WIN_SOCKET_IO_HPP__
#define __MEV_WINDOWS_WIN_SOCKET_IO_HPP__

#include "../socket_io.hpp"

#if defined(MEV_WINDOWS) && MEV_WINDOWS

namespace mev {
	typedef SOCKET socket_t;

	class socket_io;
	safeobj<socket_io> wrap_socket(socket_t socket);
}

#endif /* MEV_WINDOWS */


#endif /* __MEV_WINDOWS_WIN_SOCKET_IO_IMPL_HPP__ */
