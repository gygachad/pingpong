#pragma once

#include <vector>
#include <asio.hpp>

#include "..\str_tool.h"

#define BLOCK_SIZE 512

class connection
{	
	asio::io_service& m_io_service;
	asio::ip::tcp::socket m_sock;

public:
	using connection_ptr = std::shared_ptr<connection>;

	connection(asio::io_service& io_service):m_io_service(io_service), m_sock(io_service){}
	~connection() { disconnect(); }

	asio::ip::tcp::socket& socket() { return m_sock; }

	bool connect(const std::string& ip, const uint16_t port);
	size_t read(std::string& buffer);
	size_t read(void* data, size_t len);
	size_t write(const std::string& str);
	size_t write(const void* data, size_t len);

	void disconnect();
};