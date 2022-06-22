#include "server.h"

void server::session_th(session_ptr session)
{
	log << "Start game\r\n";
	{
		std::lock_guard<std::mutex> lock(m_ss_list_lock);
		m_session_list.push_back(session);
	}

	session->start_game();
	session->wait_end();

	log << "Client fell off\r\n";
	log << "Delete session\r\n";

	{
		std::lock_guard<std::mutex> lock(m_ss_list_lock);
		m_session_list.remove(session);
	}
}

void server::accept_handler(const std::error_code& error,
							connection_ptr new_client,
							asio::ip::tcp::acceptor& acceptor)
{
	if (error)
		return;

	log << "New client connected\r\n";

	{
		std::unique_lock<std::mutex> lock(m_cl_lock);

		if (m_create_session)
		{
			session_ptr new_session = make_shared<srv_session>(m_wait_client, new_client);

			std::thread th = std::thread(&server::session_th, this, new_session);
			th.detach();

			log << "Create new game session\r\n";
			m_create_session = false;
		}
		else
		{
			m_wait_client = new_client;

			log << "New client start waiting\r\n";
			m_create_session = true;
		}
	}

	start_accept(acceptor);
}

void server::start_accept(asio::ip::tcp::acceptor& acc)
{
	connection_ptr new_client = make_shared<connection>(m_io_service);

	acc.async_accept(new_client->socket(), std::bind(&server::accept_handler, this,
		std::placeholders::_1,
		new_client,
		std::ref(acc)));
}

void server::server_thread()
{
	asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), m_port);
	asio::ip::tcp::acceptor acc(m_io_service, ep);

	start_accept(acc);
	m_io_service.run();
}

server::~server()
{
	if (m_started)
		stop();
}

void server::set_verbose_out(bool enable)
{
	if (enable)
		log.enable();
	else
		log.disable();
}

bool server::start()
{
	m_started = true;

	log << "Start server thread\r\n";
	m_server_th = std::thread(&server::server_thread, this);
	return true;
}

void server::stop()
{
	log << "Stop server thread\r\n";
	log << "Close client connections\r\n";

	m_io_service.stop();
	m_server_th.join();

	{
		std::lock_guard<std::mutex> lock(m_cl_lock);
		m_wait_client->disconnect();
	}

	while (true)
	{
		session_ptr session;
		thread_ptr session_th;

		{
			std::lock_guard<std::mutex> lock(m_ss_list_lock);

			if (m_session_list.empty())
				break;

			session = m_session_list.front();
		}

		session->stop_game();
		//Wait session_th???
	}

	m_started = false;
}