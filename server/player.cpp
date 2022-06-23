#include "../server/player.h"

player::player(connection_ptr client_conn, const std::string& player_name, const std::string& shadow_player_name)
{
	m_player_name = player_name;
	m_shadow_player_name = shadow_player_name;
	m_connection = client_conn;

	m_view = make_shared<network_view>(client_conn);
	m_model = make_shared<model>(m_view);

	//Init gui
	m_model->create_primitive<rectangle>("battlefield", MAIN_FIELD_X, MAIN_FIELD_Y, MAIN_FIELD_W, MAIN_FIELD_H);
	m_model->create_primitive<line>("bar", MAIN_BAR_X, MAIN_BAR_Y, BAR_LEN, '=');
	m_model->create_primitive<line>("shadow_bar", SHADOW_BAR_X, SHADOW_BAR_Y, BAR_LEN, '=');
	m_model->create_primitive<point>("ball", MAIN_BALL_X, MAIN_BALL_Y, 'O');
	m_model->create_primitive<text_box>("player_state", MAIN_PLAYERNAME_FIELD_X, MAIN_PLAYERNAME_FIELD_Y, "connected");
	m_model->create_primitive<text_box>("shadow_player_state", SHADOW_PLAYER_NAME_FIELD_X, SHADOW_PLAYER_NAME_FIELD_Y, "connected");
}

void player::set_player_text(const std::string& new_text, const std::string& field_name)
{
	auto old_primitive = m_model->get_primitive(field_name);

	m_model->create_primitive<text_box>(field_name,
		old_primitive->get_x(),
		old_primitive->get_y(),
		new_text);
}

void player::set_name(const std::string& player_name)
{
	m_player_name = player_name;
	set_player_text(m_player_name + ":wait", "player_state");
}

void player::set_shadow_name(const std::string& shadow_player_name)
{
	m_shadow_player_name = shadow_player_name;
	set_player_text(m_shadow_player_name + ":wait", "shadow_player_state");
}

void player::add_shadow_goal()
{
	m_shadow_goal_counter++;
	set_player_text(m_shadow_player_name + ":" + std::to_string(m_shadow_goal_counter), "shadow_player_state");
};

void player::add_goal()
{
	m_goal_counter++;
	set_player_text(m_player_name + ":" + std::to_string(m_goal_counter), "player_state");
};

void player::change_shadow_player_state(player_state state)
{
	switch (state)
	{
	case player_state::wait:
	{
		set_player_text(m_shadow_player_name + ":wait", "shadow_player_state");
		break;
	}
	case player_state::ready:
	{
		set_player_text(m_shadow_player_name + ":ready", "shadow_player_state");
		break;
	}
	case player_state::start:
	{
		set_player_text(m_shadow_player_name + ":" + std::to_string(m_goal_counter), "shadow_player_state");
		break;
	}
	default:
		break;
	}
}

void player::change_player_state(player_state state)
{
	switch (state)
	{
	case player_state::wait:
	{
		set_player_text(m_player_name + ":wait", "player_state");
		m_state.store(state);
		break;
	}
	case player_state::ready:
	{
		set_player_text(m_player_name + ":ready", "player_state");
		m_state.store(state);
		m_ready.test_and_set();
		m_ready.notify_one();
		break;
	}
	case player_state::start:
	{
		set_player_text(m_player_name + ":" + std::to_string(m_goal_counter), "player_state");
		m_state.store(state);
		break;
	}
	case player_state::stop:
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

void player::wait_for_ready()
{
	m_ready.wait(false);
}