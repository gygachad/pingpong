#pragma once

#include <sstream>
#include <thread>
#include <asio.hpp>

#include "..\connection.h"
#include "..\game_proto.h"
#include "..\mvc\controller.h"
#include "..\mvc\network_view.h"


class server;

class srv_session
{
	using controller_ptr = std::shared_ptr<controller>;
	using net_view_ptr = std::shared_ptr<network_view>;
	using connection_ptr = std::shared_ptr<connection>;

	server& m_srv;
	model m_main_mdl;

	std::thread m_paint_th;

	std::thread m_p1_input_th;
	std::thread m_p2_input_th;

	connection_ptr m_p1_client;
	connection_ptr m_p2_client;

public:
	srv_session(	connection_ptr master, connection_ptr slave, server& srv) :
					m_p1_client(master),
					m_p2_client(slave),
					m_srv(srv) {}

	void start_game(/*Game options????*/);
	void paint_th(	controller_ptr master_ctrl, controller_ptr slave_ctrl);
	void input_th(	connection_ptr m_client,
					controller_ptr m_ctrl,
					controller_ptr s_ctrl);
	void stop_game();
};