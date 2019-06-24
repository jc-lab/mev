/**
 * @file	win_socket_io_impl.cpp
 * @class   mev::win_socket_io_impl
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#include "win_socket_io_impl.hpp"

#if defined(MEV_WINDOWS) && MEV_WINDOWS

#include "mswsock.h"
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"mswsock.lib")

namespace mev {
	safeobj<socket_io> wrap_socket(mev::socket_t socket)
	{
		return windows::win_socket_io_impl::wrap_socket(socket);
	}

	namespace windows {
		safeobj<win_socket_io_impl> win_socket_io_impl::wrap_socket(SOCKET handle)
		{
			safeobj<win_socket_io_impl> obj(new win_socket_io_impl(handle));
			return obj;
		}

		win_socket_io_impl::win_socket_io_impl(SOCKET handle)
		{
			handle_ = handle;
			is_acceptance_ = false;
			is_recvfrom_ = false;
			accept_socket_ = NULL;
		}
		win_socket_io_impl::~win_socket_io_impl()
		{
			event_closed(NULL);
		}
		void* win_socket_io_impl::io_handle() const
		{
			return (void*)handle_;
		}

		int win_socket_io_impl::start_accept(void* platform_event_ctx)
		{
			int nrst;
			DWORD dwFlags = 0;
			LPOVERLAPPED pov = (LPOVERLAPPED)platform_event_ctx;
			is_acceptance_ = true;
			d_recv_buf_.resize(sizeof(sockaddr_storage) * 2);
			accept_socket_ = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
			nrst = ::AcceptEx(handle_, accept_socket_, &d_recv_buf_[0], 0, sizeof(sockaddr_storage), sizeof(sockaddr_storage), &d_recv_tfbytes_, pov);
			return nrst;
		}

		int win_socket_io_impl::start_recv(void* platform_event_ctx, int buf_size)
		{
			int nrst;
			DWORD dwFlags = 0;
			LPOVERLAPPED pov = (LPOVERLAPPED)platform_event_ctx;
			if (buf_size > 0)
			{
				d_recv_buf_.resize(buf_size);
			}
			else if (d_recv_buf_.size() == 0) {
				d_recv_buf_.resize(8192);
			}
			is_recvfrom_ = false;
			w_recv_buf_.buf = (CHAR*)& d_recv_buf_[0];
			w_recv_buf_.len = d_recv_buf_.size();
			nrst = ::WSARecv(handle_, &w_recv_buf_, 1, &d_recv_tfbytes_, &dwFlags, pov, NULL);
			return nrst;
		}

		int win_socket_io_impl::start_recvfrom(void* platform_event_ctx, int buf_size)
		{
			int nrst;
			DWORD dwFlags = 0;
			LPOVERLAPPED pov = (LPOVERLAPPED)platform_event_ctx;
			if (buf_size > 0)
			{
				d_recv_buf_.resize(buf_size);
			}
			else if (d_recv_buf_.size() == 0) {
				d_recv_buf_.resize(8192);
			}
			is_recvfrom_ = true;
			w_recv_buf_.buf = (CHAR*)& d_recv_buf_[0];
			w_recv_buf_.len = d_recv_buf_.size();
			d_recv_from_len_ = sizeof(d_recv_from_stor_);
			nrst = ::WSARecvFrom(handle_, &w_recv_buf_, 1, &d_recv_tfbytes_, &dwFlags, (sockaddr*)&d_recv_from_stor_, &d_recv_from_len_, pov, NULL);
			return nrst;
		}

		int win_socket_io_impl::start_send(void* platform_event_ctx, const void* data_ptr, int data_size)
		{
			int nrst;
			DWORD dwFlags = 0;
			LPOVERLAPPED pov = (LPOVERLAPPED)platform_event_ctx;
			WSABUF wsabuf;
			DWORD dwTransfferedBytes = 0;
			wsabuf.buf = (CHAR*)data_ptr;
			wsabuf.len = data_size;
			nrst = ::WSASend(handle_, &wsabuf, 1, &dwTransfferedBytes, 0, pov, NULL);
			return nrst;
		}

		void win_socket_io_impl::event_closed(loop* _loop)
		{
			if (accept_socket_) {
				::closesocket(accept_socket_);
				accept_socket_ = NULL;
			}
			if (handle_) {
				::shutdown(handle_, SD_BOTH);
				::closesocket(handle_);
				handle_ = NULL;
			}
		}

		int win_socket_io_impl::mev_event(const safeobj<event_source>& evt_src, event_handler* user_handler, loop* _loop, event_type evt_type, void* userctx, event_info* evt_info)
		{
			int rc = -1;
			if ((evt_type & EVENT_READ) && is_acceptance_)
			{
				const unsigned char* bufptr = &d_recv_buf_[0];
				socket_accept_event_info evtinfo;
				evtinfo.client_socket = accept_socket_;
				GetAcceptExSockaddrs((PVOID)bufptr, 0, sizeof(sockaddr_storage), sizeof(sockaddr_storage), &evtinfo.local_addr, &evtinfo.local_addr_len, &evtinfo.remote_addr, &evtinfo.remote_addr_len);
				rc = user_handler->mev_event(_loop, evt_src, evt_type, userctx, &evtinfo);
				if (handle_) {
					accept_socket_ = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
					rc = start_accept(evt_info->platform_event_ctx);
				}
			}
			else if ((evt_type & EVENT_READ) && !is_recvfrom_) {
				socket_recv_event_info evtinfo;
				evtinfo.transferred_size = evt_info->transferred_size;
				evtinfo.recv_buf = (const char*)& d_recv_buf_[0];
				evtinfo.recv_size = evt_info->transferred_size;
				rc = user_handler->mev_event(_loop, evt_src, evt_type, userctx, &evtinfo);
				if (rc == 0) {
					if ((!(evt_type & EVENT_ONCE)) && handle_)
						rc = start_recv(evt_info->platform_event_ctx);
				}
			}
			else if ((evt_type & EVENT_READ) && is_recvfrom_) {
				socket_recvfrom_event_info evtinfo;
				evtinfo.transferred_size = evt_info->transferred_size;
				evtinfo.recv_buf = (const char*)& d_recv_buf_[0];
				evtinfo.recv_size = evt_info->transferred_size;
				evtinfo.remote_addr = (sockaddr*)&d_recv_from_stor_;
				evtinfo.remote_addr_len = d_recv_from_len_;
				rc = user_handler->mev_event(_loop, evt_src, evt_type, userctx, &evtinfo);
				if (rc == 0) {
					if ((!(evt_type & EVENT_ONCE)) && handle_)
						rc = start_recvfrom(evt_info->platform_event_ctx);
				}
			}
			else if (evt_type & EVENT_WRITE)
			{
				socket_send_event_info evtinfo;
				evtinfo.transferred_size = evt_info->transferred_size;
				if(user_handler)
					rc = user_handler->mev_event(_loop, evt_src, evt_type, userctx, &evtinfo);
			}
			return rc;
		}
	}
}

#endif


