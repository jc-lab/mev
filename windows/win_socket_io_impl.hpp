/**
 * @file	win_socket_io_impl.hpp
 * @class   mev::windows::win_socket_io_impl
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/176 )
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */
#ifndef __MEV_WINDOWS_WIN_SOCKET_IO_IMPL_HPP__
#define __MEV_WINDOWS_WIN_SOCKET_IO_IMPL_HPP__

#include "../mev_common.hpp"

#if defined(MEV_WINDOWS) && MEV_WINDOWS

#include <WinSock2.h>
#include "../socket_io.hpp"
#include "win_socket_io.hpp"

#include <vector>

namespace mev {
	namespace windows {
		class win_socket_io_impl : public socket_io {
		private:
			SOCKET handle_;
			SOCKET accept_socket_;
			
			WSABUF w_recv_buf_;
			std::vector<unsigned char> d_recv_buf_;
			DWORD d_recv_tfbytes_; // transferred bytes
			sockaddr_storage d_recv_from_stor_;
			INT d_recv_from_len_;

			DWORD d_send_tfbytes_; // transferred bytes

			bool is_acceptance_;
			bool is_recvfrom_;

		public:
			static safeobj<win_socket_io_impl> wrap_socket(SOCKET handle);

			win_socket_io_impl(SOCKET handle);
			~win_socket_io_impl();
			void* io_handle() const override;
			int start_accept(void* platform_event_ctx) override;
			int start_recv(void* platform_event_ctx, int buf_size = -1) override;
			int start_recvfrom(void* platform_event_ctx, int buf_size = -1) override;
			int start_send(void* platform_event_ctx, const void* data_ptr, int data_size) override;

			void event_closed(loop* _loop) override;
			int mev_event(const safeobj<event_source>& evt_src, event_handler* user_handler, loop* _loop, event_type evt_type, void* userctx, event_info* evt_info) override;
		};
	}
}

#endif

#endif /* __MEV_WINDOWS_WIN_SOCKET_IO_IMPL_HPP__ */
