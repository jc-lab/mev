/**
 * @file	io_socket.hpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @class	mev::io_socket
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#ifndef __MEV_IO_SOCKET_HPP__
#define __MEV_IO_SOCKET_HPP__

#include "mev_common.hpp"
#include "io.hpp"
#include "event.hpp"

#if defined(MEV_WINDOWS) && MEV_WINDOWS
#include <winsock2.h>
#include "windows/win_socket_io.hpp"
#endif

namespace mev {
	class loop;
	class socket_io;

	class socket_accept_handler
	{
	public:
		virtual int mev_socket_accept(loop* _loop, socket_io* evt_src, void* userctx, socket_t socket, sockaddr* remote_addr, int remote_addr_len, sockaddr* local_addr, int local_addr_len) = 0;
	};

	struct socket_accept_event_info : public event_info
	{
		socket_t  client_socket;
		sockaddr *remote_addr;
		int       remote_addr_len;
		sockaddr *local_addr;
		int       local_addr_len;
	};

	struct socket_recv_event_info : public event_info
	{
		int         recv_size;
		const char* recv_buf;
	};

	struct socket_recvfrom_event_info : public event_info
	{
		sockaddr* remote_addr;
		int       remote_addr_len;
		int         recv_size;
		const char* recv_buf;
	};

	struct socket_send_event_info : public event_info
	{
	};

	class socket_io : public io {
	public:
		/*
		 * If used auto acceptance should set accept_handler.
		 * 
		 * accept_handler's event_info type is socket_accept_event_info. 
		 * 
		 * sample code: 
		 *     void* platform_event_ctx = NULL;
		 *     loop->add_event(server_io, mev::EVENT_READ /=> should EVENT_READ for accept/, server_io.get() /=> use auto acceptance/, userptr, &platform_event_ctx);
		 *     server_io->start_accept(platform_event_ctx, &accept_handler);
		 */
		virtual int start_accept(void* platform_event_ctx) = 0;

		/*
		 * event_handler's event_info type is socket_recv_event_info. 
		 */
		virtual int start_recv(void* platform_event_ctx, int buf_size = -1) = 0;
		/*
		 * event_handler's event_info type is socket_recvfrom_event_info.
		 */
		virtual int start_recvfrom(void* platform_event_ctx, int buf_size = -1) = 0;
		/*
		 *
		 */
		virtual int start_send(void* platform_event_ctx, const void *data_ptr, int data_size) = 0;
	};
}

#endif /* __MEV_IO_HPP__ */
