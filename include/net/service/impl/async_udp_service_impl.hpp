// Copyright 2025 Kevin Exton
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/**
 * @file async_udp_service_impl.hpp
 * @brief This file declares an asynchronous udp service.
 */
#pragma once
#ifndef CPPNET_ASYNC_UDP_SERVICE_IMPL_HPP
#define CPPNET_ASYNC_UDP_SERVICE_IMPL_HPP
#include "net/service/async_udp_service.hpp"
namespace net::service {

template <typename UDPStreamHandler, std::size_t Size>
template <typename T>
async_udp_service<UDPStreamHandler, Size>::async_udp_service(
    socket_address<T> address) noexcept
    : address_{address}
{}

template <typename UDPStreamHandler, std::size_t Size>
auto async_udp_service<UDPStreamHandler, Size>::signal_handler(
    int signum) noexcept -> void
{
  if (signum == terminate)
    stop_();
}

template <typename UDPStreamHandler, std::size_t Size>
auto async_udp_service<UDPStreamHandler, Size>::start(
    async_context &ctx) noexcept -> std::error_code
{
  auto sock = socket_handle(address_->sin6_family, SOCK_DGRAM, 0);
  if (auto error = initialize_(sock))
    return error;

  server_sockfd_ = static_cast<socket_type>(sock);

  submit_recv(ctx, ctx.poller.emplace(std::move(sock)),
              std::make_shared<read_context>());

  return {};
}

template <typename UDPStreamHandler, std::size_t Size>
auto async_udp_service<UDPStreamHandler, Size>::submit_recv(
    async_context &ctx, const socket_dialog &socket,
    std::shared_ptr<read_context> rctx) -> void
{
  using namespace stdexec;
  using namespace io::socket;

  sender auto recvmsg =
      io::recvmsg(socket, rctx->msg, 0) |
      then([&, socket, rctx](auto &&len) mutable {
        using size_type = std::size_t;

        auto buf = std::span{rctx->buffer.data(), static_cast<size_type>(len)};
        emit(ctx, socket, std::move(rctx), buf);
      }) |
      upon_error([&, socket](auto &&error) { emit(ctx, socket); });

  ctx.scope.spawn(std::move(recvmsg));
}

template <typename UDPStreamHandler, std::size_t Size>
auto async_udp_service<UDPStreamHandler, Size>::emit(
    async_context &ctx, const socket_dialog &socket,
    std::shared_ptr<read_context> rctx, std::span<const std::byte> buf) -> void
{
  static_cast<UDPStreamHandler *>(this)->service(ctx, socket, std::move(rctx),
                                                 buf);
}

template <typename UDPStreamHandler, std::size_t Size>
[[nodiscard]] auto async_udp_service<UDPStreamHandler, Size>::initialize_(
    const socket_handle &socket) -> std::error_code
{
  using namespace io;
  using namespace io::socket;

  if (auto reuse = socket_option<int>(1);
      setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, reuse))
  {
    return {errno, std::system_category()};
  }

  if constexpr (requires(UDPStreamHandler handler) {
                  {
                    handler.initialize(socket)
                  } -> std::same_as<std::error_code>;
                })
  {
    if (auto error = static_cast<UDPStreamHandler *>(this)->initialize(socket))
      return error;
  }

  if (bind(socket, address_))
    return {errno, std::system_category()};

  address_ = getsockname(socket, address_);

  return {};
}

template <typename UDPStreamHandler, std::size_t Size>
auto async_udp_service<UDPStreamHandler, Size>::stop_() -> void
{
  using namespace io::socket;
  auto sockfd = server_sockfd_.exchange(INVALID_SOCKET);
  shutdown(sockfd, SHUT_RD);
}

} // namespace net::service
#endif // CPPNET_ASYNC_UDP_SERVICE_IMPL_HPP
