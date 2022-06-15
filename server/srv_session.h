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
	ready,
	start,
	stop
};

class player
{
	using connection_ptr = std::shared_ptr<connection>;
	using net_view_ptr = std::shared_ptr<network_view>;
	using model_ptr = std::shared_ptr<model>;

	connection_ptr m_connection;
	net_view_ptr m_player_view;
	model_ptr m_player_model;

	std::atomic_flag m_ready;
	std::string m_player_name;
	std::string m_shadow_player_name;

	size_t m_goal_counter = 0;
	size_t m_shadow_goal_counter = 0;

	void set_player_text(const std::string& new_text, const std::string& field_name)
	{
		auto old_primitive = m_player_model->get_primitive(field_name);

		m_player_model->create_primitive<text_box>(	field_name,
													old_primitive->get_x(),
													old_primitive->get_y(),
													new_text);
	}

public:
	std::atomic<game_state> m_state;

	player(connection_ptr client_conn, const std::string& player_name, const std::string& shadow_player_name)
	{
		m_player_name = player_name;
		m_shadow_player_name = shadow_player_name;
		m_connection = client_conn;

		m_player_view = make_shared<network_view>(client_conn);
		m_player_model = make_shared<model>(m_player_view);

		//Init gui
		m_player_model->create_primitive<rectangle>("battlefield", MAIN_FIELD_X, MAIN_FIELD_Y, MAIN_FIELD_W, MAIN_FIELD_H);
		m_player_model->create_primitive<line>("bar", MAIN_BAR_X, MAIN_BAR_Y, BAR_LEN, '=');
		m_player_model->create_primitive<line>("shadow_bar", SHADOW_BAR_X, SHADOW_BAR_Y, BAR_LEN, '=');
		m_player_model->create_primitive<text_box>("player_state", MAIN_PLAYERNAME_FIELD_X, MAIN_PLAYERNAME_FIELD_Y, "connected");
		m_player_model->create_primitive<text_box>("shadow_player_state", SHADOW_PLAYER_NAME_FIELD_X, SHADOW_PLAYER_NAME_FIELD_Y, "connected");
	}

	model_ptr get_model() { return m_player_model; }
	connection_ptr get_connection() { return m_connection; }

	std::string& get_name() { return m_player_name; }
	void set_name(const std::string& player_name) 
	{ 
		m_player_name = player_name;
		set_player_text(m_player_name + ":wait", "player_state");
	}

	std::string& get_shadow_name() { return m_shadow_player_name; }
	void set_shadow_name(const std::string& shadow_player_name) 
	{ 
		m_shadow_player_name = shadow_player_name; 
		set_player_text(m_shadow_player_name + ":wait", "shadow_player_state");
	}

	void add_shadow_goal()
	{
		m_shadow_goal_counter++;
		set_player_text(m_shadow_player_name + ":" + std::to_string(m_shadow_goal_counter), "shadow_player_state");
	};

	void add_goal() 
	{ 
		m_goal_counter++;
		set_player_text(m_player_name + ":" + std::to_string(m_goal_counter), "player_state");
	};

	void change_shadow_player_state(game_state state)
	{
		switch (state)
		{
			case game_state::wait:
			{	
				set_player_text(m_shadow_player_name + ":wait", "shadow_player_state");
				break;
			}
			case game_state::ready:
			{	
				set_player_text(m_shadow_player_name + ":ready", "shadow_player_state");
				break;
			}
			case game_state::start:
			{
				set_player_text(m_shadow_player_name + ":" + std::to_string(m_goal_counter), "shadow_player_state");
				break;
			}
			default:
				break;
		}
	}

	void change_player_state(game_state state)
	{
		switch (state)
		{
			case game_state::wait:
			{
				set_player_text(m_player_name + ":wait", "player_state");
				m_state.store(state);
				break;
			}
			case game_state::ready:
			{
				set_player_text(m_player_name + ":ready", "player_state");
				m_state.store(state);
				m_ready.test_and_set();
				m_ready.notify_one();
				break;
			}
			case game_state::start:
			{
				set_player_text(m_player_name + ":" + std::to_string(m_goal_counter), "player_state");
				m_state.store(state);
				break;
			}
			case game_state::stop:
			{
				m_ready.test_and_set();
				m_ready.notify_one();
				m_state.store(state);
				break;
			}
			default:
				break;
		}
	}

	void wait_for_ready()
	{
		m_ready.wait(false);
	}
};

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
	std::atomic<game_state> m_state;

	connection_ptr m_p1_client;
	connection_ptr m_p2_client;

	player_ptr m_player1;
	player_ptr m_player2;

public:
	srv_session(connection_ptr master, connection_ptr slave);

	void paint_th(	player_ptr player1, player_ptr player2);
	void input_th(	player_ptr player1, player_ptr player2, bool master);

	void start_game();
	void wait_end();
	void stop_game();

	~srv_session() {}
};