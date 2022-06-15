#pragma once

#include <sstream>
#include <thread>
#include <atomic>

#include <asio.hpp>

#include "..\connection.h"
#include "..\mvc\network_view.h"
#include "..\mvc\model.h"
#include "..\game_config.h"

enum class game_state
{
	wait,
	start,
	stop
};

class srv_session
{
	using model_ptr = std::shared_ptr<model>;
	using net_view_ptr = std::shared_ptr<network_view>;
	using connection_ptr = std::shared_ptr<connection>;
	using session_ptr = std::shared_ptr<srv_session>;

	std::thread m_paint_th;
	std::thread m_p1_input_th;
	std::thread m_p2_input_th;

	connection_ptr m_p1_client;
	connection_ptr m_p2_client;
	
	//Syncronization
	std::atomic_flag m_p1_ready;
	std::atomic_flag m_p2_ready;
	std::atomic_flag m_stop_game;
	std::atomic<game_state> m_state;

public:
	srv_session(	connection_ptr master, connection_ptr slave) :
					m_p1_client(master),
					m_p2_client(slave) {}

	void init_gui(	model_ptr model);
	void paint_th(	model_ptr p1_model, model_ptr p2_model);
	void input_th(	connection_ptr m_client,
					model_ptr p1_model,
					model_ptr p2_model, bool master);

	void start_game();
	void wait_end();
	void stop_game();

	~srv_session() {}
};