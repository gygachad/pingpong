#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <list>
#include <map>

#include <asio.hpp>

#include "../server/srv_session.h"
#include "../connection.h"

class logger
{
	bool m_enabled = false;

public:
	logger() {}

	void enable() { m_enabled = true; }
	void disable() { m_enabled = false; }

	logger& operator<<(const std::string& data)
	{
		if (m_enabled)
			std::cout << data;

		return *this;
	}
};

class ICmd_dispatcher
{
public:
	virtual std::string execute(std::string& cmd_line) = 0;

	~ICmd_dispatcher() {}
};


class server
{
	using socket = asio::ip::tcp::socket;
	using socket_ptr = std::shared_ptr<socket>;
	using session_ptr = std::shared_ptr<srv_session>;
	using connection_ptr = std::shared_ptr<connection>;
	using thread_ptr = std::shared_ptr<std::thread>;

	//std::list<connection_ptr> m_wait_client_list;
	connection_ptr m_wait_client;
	std::mutex m_cl_lock;
	bool m_create_session = false;

	std::map<session_ptr, std::thread> m_session_pool;

	std::list<session_ptr> m_session_list;
	std::mutex m_ss_list_lock;

	asio::io_service& m_io_service;
	uint16_t m_port;

	std::thread m_server_th;
	logger log;

	bool m_started = false;

	void session_th(session_ptr session);

	void accept_handler(const std::error_code& error,
						connection_ptr new_client,
						asio::ip::tcp::acceptor& acceptor);

	void start_accept(asio::ip::tcp::acceptor& acc);

	void server_thread();

public:
	server(asio::io_service& io_service, uint16_t port) : m_io_service(io_service), m_port(port) {}
	~server();

	void set_verbose_out(bool enable);

	bool start();
	void stop();
};