#pragma once

#include <vector>
#include <asio.hpp>

#include "..\str_tool.h"

#define BLOCK_SIZE 512

class connection
{	
	asio::io_service& m_io_service;
	asio::ip::tcp::socket m_sock;

	bool m_connected = false;

public:
	using connection_ptr = std::shared_ptr<connection>;

	connection(asio::io_service& io_service):m_io_service(io_service), m_sock(io_service){}
	~connection() {}

	asio::ip::tcp::socket& socket() { return m_sock; }

	bool connect(const std::string& ip, const uint16_t port);
	size_t read(std::string& buffer);
	size_t write(const std::string& str);
	void disconnect();
};