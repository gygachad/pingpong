#pragma once

#include "..\connection.h"
#include "..\mvc\network_view.h"
#include "..\mvc\model.h"
#include "..\game_config.h"

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

	void set_player_text(const std::string& new_text, const std::string& field_name);

public:
	std::atomic<game_state> m_state;

	player(connection_ptr client_conn, const std::string& player_name, const std::string& shadow_player_name);

	model_ptr get_model() { return m_player_model; }
	connection_ptr get_connection() { return m_connection; }

	std::string& get_name();
	void set_name(const std::string& player_name);

	std::string& get_shadow_name();
	void set_shadow_name(const std::string& shadow_player_name);
	
	void add_shadow_goal();
	void add_goal();
	

	void change_shadow_player_state(game_state state);
	void change_player_state(game_state state);
	void wait_for_ready();
};