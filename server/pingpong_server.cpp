#include "pingpong_server.h"

void pingpong_server::accept_handler(	const error_code& error,
										socket_ptr sock,
										asio::ip::tcp::acceptor& acceptor)
{
	if (error)
		return;

	log << "New client connected\r\n";

	client_ptr new_client = make_shared<pingpong_client>();
	new_client->accept(sock);

	session_ptr new_session;

	{
		lock_guard<mutex> lock(m_cl_lock);
		if (m_create_new_session)
		{
			new_session = make_shared<srv_game_session>(m_wait_client, new_client, *this);
			m_create_new_session = false;
			log << "Create new game session\r\n";
		}
		else
		{
			m_wait_client = new_client;
			m_create_new_session = true;
			log << "New client start waiting\r\n";
		}
	}

	//Add session and start thread
	if(m_create_new_session == false)
	{
		{
			lock_guard<mutex> lock(m_ss_list_lock);
			m_sesssion_list.push_back(new_session);
		}

		//Pass game options
		new_session->start_game();
	}

	start_accept(acceptor);
}

void pingpong_server::start_accept(asio::ip::tcp::acceptor& acc)
{
	socket_ptr sock = make_shared<socket>(m_io_context);

	acc.async_accept(	*sock, std::bind(&pingpong_server::accept_handler, this,
						std::placeholders::_1,
						sock,
						std::ref(acc)));
}

void pingpong_server::server_thread()
{
	asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), m_port);
	asio::ip::tcp::acceptor acc(m_io_context, ep);

	start_accept(acc);
	m_io_context.run();
}

pingpong_server::pingpong_server(uint16_t port)
{
	m_port = port;
}

pingpong_server::~pingpong_server()
{
	if (m_started)
		stop();
}

void pingpong_server::set_verbose_out(bool enable)
{
	if (enable)
		log.enable();
	else
		log.disable();
}

bool pingpong_server::start()
{
	m_started = true;

	log << "Start server thread\r\n";
	m_server_th = thread(&pingpong_server::server_thread, this);
	return true;
}

void pingpong_server::stop()
{
	log << "Stop server thread\r\n";

	m_io_context.stop();
	m_server_th.join();

	log << "Close client connections\r\n";

	client_ptr client;

	if(m_create_new_session)
	{
		lock_guard<mutex> lock(m_cl_lock);
		m_wait_client->disconnect();
	}

	while (true)
	{
		session_ptr session;

		{
			lock_guard<mutex> lock(m_ss_list_lock);

			if (m_sesssion_list.empty())
				break;

			session = m_sesssion_list.front();
		}

		log << "Shutdown client\r\n";

		session->stop_game();
	}

	m_started = false;
}