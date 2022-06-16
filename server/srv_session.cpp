﻿#include <ctime>
#include <climits>

#include "..\game_config.h"
#include "srv_session.h"

srv_session::srv_session(connection_ptr master, connection_ptr slave)
{
	m_p1_client = master;
	m_p2_client = slave;

	m_player1 = make_shared<player>(m_p1_client, "player1", "player2");
	m_player2 = make_shared<player>(m_p2_client, "player2", "player1");
}

void srv_session::start_game()
{
	m_state.store(game_state::wait);

	m_paint_th = std::thread(&srv_session::paint_th, this, m_player1, m_player2);
	m_p1_input_th = std::thread(&srv_session::input_th, this, m_player1, m_player2, true);
	m_p2_input_th = std::thread(&srv_session::input_th, this, m_player2, m_player1, false);
}

void srv_session::input_th(player_ptr player1, player_ptr player2, bool master)
{
	std::string buffer;
	std::string custom_player_name = "";

	auto p1_model = player1->get_model();
	auto p2_model = player2->get_model();

	auto player_connection = player1->get_connection();

	player1->change_player_state(game_state::wait);
	player1->change_shadow_player_state(game_state::wait);

	size_t battlefield_x = p1_model->get_primitive("battlefield")->get_x();
	size_t battlefield_w = p1_model->get_primitive("battlefield")->get_w();

	int x_step = 0;

	while (true)
	{
		//Check if socket alive
		size_t len = player_connection->read(buffer);
		
		if (len == 0)
		{
			stop_game();
			break;
		}

		std::vector<std::string> input_vec = str_tool::split(buffer, "\n");

		for (const std::string& cmd : input_vec)
		{
			std::stringstream ss(cmd);

			int key_code = 0;
			ss >> key_code;

			switch (key_code)
			{
				case KEY_LEFT:
				{
					if (p1_model->get_primitive("bar")->get_x() > battlefield_x + 1)
						x_step = -1;
					else
						continue;

					break;
				}
				case KEY_RIGHT:
				{
					if (p1_model->get_primitive("bar")->get_x() < battlefield_w - BAR_LEN - 1)
						x_step = 1;
					else
						continue;

					break;
				}
				case KEY_SPACEBAR:
				{
					player2->change_shadow_player_state(game_state::ready);
					player1->change_player_state(game_state::ready);
					continue;
				}
				default:
				{
					if (player1->m_state.load() == game_state::wait)
					{
						if (custom_player_name.length() < 10)
						{
							if (key_code >= 'a' && key_code <= 'z')
							{
								custom_player_name += char(key_code);

								//update names on screen
								player1->set_name(custom_player_name);
								player2->set_shadow_name(custom_player_name);
							}
						}
					}
					
					continue;
				}
			}

			p1_model->move_primitive("bar", x_step, 0);
			//Mirror shadow primitive moves
			p2_model->move_primitive("shadow_bar", -x_step, 0);

			if (master)
			{
				//Move ball until game not started
				if (m_state.load() != game_state::start)
				{
					p1_model->move_primitive("ball", x_step, 0);
					//Mirror shadow primitive moves
					p2_model->move_primitive("shadow_ball", -x_step, 0);
				}
			}
		}
	}
}

void srv_session::paint_th(player_ptr player1, player_ptr player2)
{
	std::string buffer;

	std::srand(std::time(nullptr));
	//int x_step = (std::rand() % 2) ? 1 : -1;
	int x_step = -1;
	int y_step = -1;

	auto p1_model = player1->get_model();
	auto p2_model = player2->get_model();

	p1_model->create_primitive<point>("ball", MAIN_BALL_X, MAIN_BALL_Y, 'O');
	p2_model->create_primitive<point>("shadow_ball", SHADOW_BALL_X, SHADOW_BALL_Y, 'O');

	//Wait for players ready
	player1->wait_for_ready();
	player2->wait_for_ready();

	//Player fell of before start
	if (player1->m_state.load() == game_state::stop ||
		player2->m_state.load() == game_state::stop)
		return;

	//Add graphic counter
	//std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(3000));

	player1->change_player_state(game_state::start);
	player2->change_player_state(game_state::start);
	player1->change_shadow_player_state(game_state::start);
	player2->change_shadow_player_state(game_state::start);

	size_t x = p1_model->get_primitive("ball")->get_x();
	size_t y = p1_model->get_primitive("ball")->get_y();
	size_t shadow_x = p2_model->get_primitive("shadow_ball")->get_x();

	m_state.store(game_state::start);

	auto p1_bar = p1_model->get_primitive("bar");
	auto p2_bar = p2_model->get_primitive("bar");

	bool change_angle = false;
	bool p1_pass = true;
	bool p2_pass = true;

	while (true)
	{
		//Check game status
		if (m_state.load() == game_state::stop)
			break;

		if (int(x + x_step) > int(MAIN_FIELD_W - 2))
			x_step = -1;

		if (int(x + x_step) <= int(MAIN_FIELD_X))
			x_step = 1;

		x += x_step;
		y += y_step;
		shadow_x -= x_step;

		size_t bar_x = 0;
		size_t bar_w = 0;

		if (y == MAIN_FIELD_Y + 1)
		{
			bar_x = p2_bar->get_x();
			bar_w = p2_bar->get_w();

			if ((shadow_x < bar_x) || (shadow_x >= bar_x + bar_w))
			{
				if (p1_pass)
				{
					player2->add_goal();
					player1->add_shadow_goal();
				}
				else
					p1_pass = true;
			}
			else
			{
				change_angle = true;
				y_step = 1;
			}
		}

		if (y == MAIN_FIELD_H - 4)
		{
			bar_x = p1_bar->get_x();
			bar_w = p1_bar->get_w();

			if ((x < bar_x) || (x >= bar_x + bar_w))
			{
				if (p2_pass)
				{
					player1->add_goal();
					player2->add_shadow_goal();
				}
				else
					p2_pass = true;
			}
			else
			{
				change_angle = true;
				y_step = -1;
			}
		}

		if (y == MAIN_FIELD_Y)
		{
			y_step = 1;
			p1_pass = false;
		}
		if (y == MAIN_FIELD_H - 3)
		{
			y_step = -1;
			p2_pass = false;
		}

		//Change ball angle if ball hit bar corners
		/*
		if (change_angle)
		{
			if ((bar_x + 1 == x) || (bar_x + bar_w - 2 == x))
			{
				if (x >= 2 && MAIN_FIELD_W - 2)
					x_step = x_step * 2;
			}
			if ((bar_x == x) || (bar_x + bar_w - 1 == x))
			{
				if (x >= 3 && x < MAIN_FIELD_W - 3)
					x_step = x_step * 3;
			}
			change_angle = false;
		}
		*/

		p1_model->move_primitive("ball", x_step, y_step);
		//Mirror shadow primitive moves
		p2_model->move_primitive("shadow_ball", -x_step, -y_step);

		std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(BALL_TRESHOLD));
	}
}

void srv_session::stop_game()
{
	m_state.store(game_state::stop);

	m_stop_game.test_and_set();
	m_stop_game.notify_one();
}

void srv_session::wait_end()
{
	m_stop_game.wait(false);

	m_player1->change_player_state(game_state::stop);
	m_player2->change_player_state(game_state::stop);

	m_state.store(game_state::stop);

	m_p1_client->disconnect();
	m_p2_client->disconnect();

	if (m_paint_th.joinable())
		m_paint_th.join();
	
	if (m_p1_input_th.joinable())
		m_p1_input_th.join();
	
	if (m_p2_input_th.joinable())
		m_p2_input_th.join();
}