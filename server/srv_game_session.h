#pragma once

#include <sstream>
#include <thread>
#include <asio.hpp>

#include "..\pingpong_client.h"
#include "pingpong_server.h"
#include "..\game_proto.h"

#include "..\mvc\controller.h"
#include "..\mvc\network_view.h"
#include "..\pingpong_client.h"

class pingpong_server;

class srv_game_session
{
	pingpong_server& m_srv;
	model m_main_mdl;

	std::thread m_paint_th;

	std::thread m_p1_input_th;
	std::thread m_p2_input_th;

	pingpong_client::client_ptr m_p1_client;
	pingpong_client::client_ptr m_p2_client;

public:
	using controller_ptr = std::shared_ptr<controller>;
	using net_view_ptr = std::shared_ptr<network_view>;

	srv_game_session(pingpong_client::client_ptr master, pingpong_client::client_ptr slave, pingpong_server& srv) :
					m_p1_client(master),
					m_p2_client(slave),
					m_srv(srv) {}

	void start_game(/*Game options????*/);
	void paint_th(controller_ptr master_ctrl, controller_ptr slave_ctrl);
	void input_th(	pingpong_client::client_ptr m_client,
					controller_ptr m_ctrl,
					controller_ptr s_ctrl);
	void stop_game();
};