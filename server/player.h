#pragma once

#include "../connection.h"
#include "../mvc/network_view.h"
#include "../mvc/model.h"
#include "../game_config.h"

class player
{
	using connection_ptr = std::shared_ptr<connection>;
	using net_view_ptr = std::shared_ptr<network_view>;
	using model_ptr = std::shared_ptr<model>;

	connection_ptr m_connection;
	net_view_ptr m_view;
	model_ptr m_model;

	std::atomic_flag m_ready;
	std::string m_player_name;
	std::string m_shadow_player_name;


	void set_player_text(const std::string& new_text, const std::string& field_name);

public:
	size_t m_goal_counter = 0;
	size_t m_shadow_goal_counter = 0;

	std::atomic<player_state> m_state = player_state::stop;

	player(connection_ptr client_conn, const std::string& player_name, const std::string& shadow_player_name);

	connection_ptr get_connection() { return m_connection; }
	std::string& get_shadow_name() { return m_shadow_player_name; }

	void set_name(const std::string& player_name);
	void set_shadow_name(const std::string& shadow_player_name);

	void add_shadow_goal();
	void add_goal();

	void set_ready_timer(size_t i) { m_model->create_primitive<text_box>("ready_timer", READY_TIMER_X, READY_TIMER_Y, std::to_string(i)); }
	void clean_ready_timer() { m_model->clean_primitive("ready_timer"); }

	void move_bar(int x_offset) { m_model->move_primitive("bar", x_offset, 0); }
	void move_shadow_bar(int x_offset) { m_model->move_primitive("shadow_bar", x_offset, 0); }
	void move_ball(int x_offset, int y_offset) { m_model->move_primitive("ball", x_offset, y_offset); }

	size_t get_bar_pos() { return m_model->get_primitive("bar")->get_x(); }
	size_t get_bar_len() { return m_model->get_primitive("bar")->get_w(); }
	size_t get_battlefield_x() { return m_model->get_primitive("battlefield")->get_x(); }
	size_t get_battlefield_w() { return m_model->get_primitive("battlefield")->get_w(); }
	size_t get_shadow_bar_pos() { return m_model->get_primitive("shadow_bar")->get_x(); }
	size_t get_ball_x() { return m_model->get_primitive("ball")->get_x(); }
	size_t get_ball_y() { return m_model->get_primitive("ball")->get_y(); }

	void change_shadow_player_state(player_state state);
	void change_player_state(player_state state);
	void wait_for_ready();
};