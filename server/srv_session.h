#pragma once

#include <sstream>
#include <thread>
#include <atomic>

#include <asio.hpp>

#include "..\connection.h"
#include "..\mvc\controller.h"
#include "..\mvc\network_view.h"
#include "..\game_config.h"

enum class game_state
{
	wait,
	start,
	end
};

class srv_session
{
	using controller_ptr = std::shared_ptr<controller>;
	using net_view_ptr = std::shared_ptr<network_view>;
	using connection_ptr = std::shared_ptr<connection>;

	std::thread m_paint_th;

	std::thread m_p1_input_th;
	std::thread m_p2_input_th;

	connection_ptr m_p1_client;
	connection_ptr m_p2_client;

	//mutex lock for main model??
	model m_gui_model;
	/*
	std::atomic_flag m_p1_ready;
	std::atomic_flag m_p2_ready;
	*/
	game_state m_state = game_state::wait;

public:
	srv_session(	connection_ptr master, connection_ptr slave) :
					m_p1_client(master),
					m_p2_client(slave) {}

	void init_gui();
	auto get_primitive(const string& name);

	void start_game(/*Game options????*/);
	void paint_th(	controller_ptr master_ctrl, controller_ptr slave_ctrl);
	void input_th(	connection_ptr m_client,
					controller_ptr m_ctrl,
					controller_ptr s_ctrl, bool master);

	void wait_end();
	void stop_game();

	~srv_session() {}
};