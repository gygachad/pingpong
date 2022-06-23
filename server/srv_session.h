#pragma once

#include <sstream>
#include <thread>
#include <atomic>
#include <asio.hpp>

#include "../connection.h"
#include "../mvc/network_view.h"
#include "../mvc/model.h"
#include "../server/player.h"
#include "../game_config.h"

class srv_session
{
	using player_ptr = std::shared_ptr<player>;
	using connection_ptr = std::shared_ptr<connection>;
	using session_ptr = std::shared_ptr<srv_session>;

	std::thread m_paint_th;
	std::thread m_p1_input_th;
	std::thread m_p2_input_th;

	//Syncronization
	std::atomic_flag m_stop_game;
	std::atomic_flag m_start_game;
	std::atomic<game_state> m_state;

	connection_ptr m_p1_client;
	connection_ptr m_p2_client;

	player_ptr m_player1;
	player_ptr m_player2;

	//Bar-Ball synchro
	std::mutex m_bb_lock;

public:
	srv_session(connection_ptr master, connection_ptr slave);

	void paint_th(player_ptr player1, player_ptr player2);
	void input_th(player_ptr player1, player_ptr player2, bool master);

	void start_game();
	void wait_end();
	void stop_game();

	~srv_session() {}
};