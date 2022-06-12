#include "server.h"

void server::session_th(session_ptr session)
{
	log << "Start game\r\n";
	{
		lock_guard<mutex> lock(m_ss_list_lock);
		m_session_list.push_back(session);
	}

	session->start_game();
	session->wait_end();

	log << "Client fell off\r\n";
	log << "Delete session\r\n";

	{
		lock_guard<mutex> lock(m_ss_list_lock);
		m_session_list.remove(session);
	}
}

void server::accept_handler(	const error_code& error,
										socket_ptr sock,
										asio::ip::tcp::acceptor& acceptor)
{
	if (error)
		return;

	log << "New client connected\r\n";

	connection_ptr new_client = make_shared<connection>();
	new_client->accept(sock);

	{
		std::unique_lock<mutex> lock(m_cl_lock);

		if (m_wait_client_list.size() >= 1)
		{
			connection_ptr wait_client = m_wait_client_list.front();
			m_wait_client_list.pop_front();

			session_ptr new_session = make_shared<srv_session>(wait_client, new_client);

			std::thread th = std::thread(&server::session_th, this, new_session);
			th.detach();

			log << "Create new game session\r\n";
		}
		else
		{
			m_wait_client_list.push_back(new_client);

			log << "New client start waiting\r\n";
		}
	}

	start_accept(acceptor);
}

void server::start_accept(asio::ip::tcp::acceptor& acc)
{
	socket_ptr sock = make_shared<socket>(m_io_service);

	acc.async_accept(	*sock, std::bind(&server::accept_handler, this,
						std::placeholders::_1,
						sock,
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
	m_server_th = thread(&server::server_thread, this);
	return true;
}

void server::stop()
{
	log << "Stop server thread\r\n";
	log << "Close client connections\r\n";

	while (true)
	{
		connection_ptr client;

		{
			lock_guard<mutex> lock(m_cl_lock);

			if (m_wait_client_list.empty())
				break;

			client = m_wait_client_list.front();
			m_wait_client_list.pop_front();
		}

		client->disconnect();
		//Wait session_thread???
	}

	while (true)
	{
		session_ptr session;
		thread_ptr session_th;

		{
			lock_guard<mutex> lock(m_ss_list_lock);

			if (m_session_list.empty())
				break;

			session = m_session_list.front();
		}

		session->stop_game();
		//Wait session_thread???
	}

	//m_io_context.stop();
	m_io_service.stop();
	m_server_th.join();

	m_started = false;
}