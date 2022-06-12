#pragma once

#include <vector>
#include <asio.hpp>

#include "..\str_tool.h"

#define BLOCK_SIZE 512

class connection
{
	using sock_ptr = std::shared_ptr<asio::ip::tcp::socket>;
	
	asio::io_context m_io_context;
	sock_ptr m_sock;

	bool m_connected = false;

public:
	using connection_ptr = std::shared_ptr<connection>;

	connection() {}
	~connection() {}

	bool connect(const std::string& ip, const uint16_t port);
	bool accept(sock_ptr sock);
	size_t read(std::string& buffer);
	size_t write(const std::string& str);
	void disconnect();
};