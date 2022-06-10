#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <list>

#include <asio.hpp>

using namespace std;

#include "..\str_tool.h"
#include "srv_game_session.h"

class srv_game_session;

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
			cout << data;

		return *this;
	}
};

class ICmd_dispatcher
{
public:
	virtual std::string execute(std::string& cmd_line) = 0;

	~ICmd_dispatcher() {}
};

class pingpong_server
{
	using socket = asio::ip::tcp::socket;
	using socket_ptr = shared_ptr<socket>;
	using client_ptr = shared_ptr<pingpong_client>;
	using session_ptr = shared_ptr<srv_game_session>;

	client_ptr m_wait_client;
	mutex m_cl_lock;
	bool m_create_new_session = false;

	list<session_ptr> m_sesssion_list;
	mutex m_ss_list_lock;

	uint16_t m_port;
	size_t m_bulk_size;

	asio::io_context m_io_context;
	thread m_server_th;

	logger log;

	bool m_started = false;

	void client_session(client_ptr client);
	
	void accept_handler(const error_code& error,
						socket_ptr sock,
						asio::ip::tcp::acceptor& acceptor);

	void start_accept(asio::ip::tcp::acceptor& acc);
	
	void server_thread();

public:
	pingpong_server(uint16_t port);
	~pingpong_server();

	void set_verbose_out(bool enable);

	bool start();
	void stop();
};