#include <ctime>

#include "..\game_config.h"
#include "srv_session.h"

void srv_session::init_gui(model_ptr model)
{
	model->create_primitive<rectangle>("battlefield",	MAIN_FIELD_X, MAIN_FIELD_Y,
														MAIN_FIELD_W, MAIN_FIELD_H);

	model->create_primitive<bar>("bar", MAIN_BAR_X, MAIN_BAR_Y, BAR_LEN);
	model->create_primitive<bar>("shadow_bar", SHADOW_BAR_X, SHADOW_BAR_Y, BAR_LEN);
	model->create_primitive<point>("ball", MAIN_BALL_X, MAIN_BALL_Y, 'O');
	model->create_primitive<point>("shadow_ball", SHADOW_BALL_X, SHADOW_BALL_Y, 'O');

	//model->create_primitive<text_box>("player1_score",	PPP_SCOREBAR_FIELD_X, PPP_SCOREBAR_FIELD_Y, "0");
	//model->create_primitive<text_box>("player2_score", PPP_SCOREBAR_FIELD_X, PPP_SCOREBAR_FIELD_Y, "0");
}

void srv_session::start_game()
{
	net_view_ptr player1_view = make_shared<network_view>(m_p1_client);
	net_view_ptr player2_view = make_shared<network_view>(m_p2_client);

	model_ptr player1_model = make_shared<model>(player1_view);
	model_ptr player2_model = make_shared<model>(player2_view);

	m_state.store(game_state::wait);

	init_gui(player1_model);
	init_gui(player2_model);

	player1_model->draw_primitive("battlefield");
	player2_model->draw_primitive("battlefield");

	m_paint_th = std::thread(&srv_session::paint_th, this, player1_model, player2_model);
	m_p1_input_th = std::thread(&srv_session::input_th, this, m_p1_client, player1_model, player2_model, true);
	m_p2_input_th = std::thread(&srv_session::input_th, this, m_p2_client, player2_model, player1_model, false);
}

void srv_session::input_th(		connection_ptr m_client, 
								model_ptr p1_model,
								model_ptr p2_model, bool master)
{
	std::string buffer;
	std::string player_name;

	if (master)
		player_name = "PLAYER1_NAME";
	else
		player_name = "PLAYER2_NAME";

	p1_model->create_primitive<text_box>("player_name", MAIN_PLAYERNAME_FIELD_X, MAIN_PLAYERNAME_FIELD_Y, player_name);
	p2_model->create_primitive<text_box>("shadow_player_name", SHADOW_PLAYER_NAME_FIELD_X, SHADOW_PLAYER_NAME_FIELD_Y, player_name);

	size_t battlefield_x = p1_model->get_primitive("battlefield")->get_x();
	size_t battlefield_w = p1_model->get_primitive("battlefield")->get_w();

	p1_model->draw_primitive("bar");
	p2_model->draw_primitive("shadow_bar");
	p1_model->draw_primitive("player_name");
	p2_model->draw_primitive("shadow_player_name");
	
	int x_step = 0;

	while (true)
	{
		size_t len = m_client->read(buffer);
		
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
				case 'E':
				{
					continue;
				}
				case KEY_SPACEBAR:
				{
					if (master)
					{
						m_p1_ready.test_and_set();
						m_p1_ready.notify_one();
					}
					else
					{
						m_p2_ready.test_and_set();
						m_p2_ready.notify_one();
					}

					continue;
				}
				default:
				{
					/*
					if (m_state.load() == game_state::wait)
					{
						if (key_code >= 'A' && key_code <= 'z')
						{
							if (player_name == "PLAYER1_NAME")
								player_name = "";
							if (player_name == "PLAYER2_NAME")
								player_name = "";

							player_name += ss.str();

							p1_model->create_primitive<text_box>("player_name", MAIN_PLAYERNAME_FIELD_X, MAIN_PLAYERNAME_FIELD_Y, player_name);
							p2_model->create_primitive<text_box>("shadow_player_name", SHADOW_PLAYER_NAME_FIELD_X, SHADOW_PLAYER_NAME_FIELD_Y, player_name);

							p1_model->draw_primitive("player_name");
							p2_model->draw_primitive("shadow_player_name");
						}
					}
					*/
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

void srv_session::paint_th(model_ptr p1_model, model_ptr p2_model)
{
	std::string buffer;

	std::srand(std::time(nullptr));
	int x_step = (std::rand() % 2) ? 1 : -1;
	int y_step = -1;

	p1_model->draw_primitive("ball");
	p2_model->draw_primitive("shadow_ball");

	//Wait for players ready
	m_p1_ready.wait(false);
	m_p2_ready.wait(false);

	m_state = game_state::start;

	size_t x = p1_model->get_primitive("ball")->get_x();
	size_t y = p1_model->get_primitive("ball")->get_y();

	while (true)
	{
		//Check game status
		if (m_state == game_state::stop)
			break;

		if (x == MAIN_FIELD_W - 2)
			x_step = -1;

		if (y == MAIN_FIELD_Y + 2)
			y_step = 1;

		if (x == MAIN_FIELD_X + 1)
			x_step = 1;

		if (y == MAIN_FIELD_H - 3)
			y_step = -1;

		x += x_step;
		y += y_step;

		p1_model->move_primitive("ball", x_step, y_step);
		//Mirror shadow primitive moves
		p2_model->move_primitive("shadow_ball", -x_step, -y_step);

		//lock
		//m_view.paint

		std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(100));
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

	m_state.store(game_state::stop);

	m_p1_ready.test_and_set();
	m_p1_ready.notify_one();
	m_p2_ready.test_and_set();
	m_p2_ready.notify_one();

	m_p1_client->disconnect();
	m_p2_client->disconnect();

	if (m_paint_th.joinable())
		m_paint_th.join();
	
	if (m_p1_input_th.joinable())
		m_p1_input_th.join();
	
	if (m_p2_input_th.joinable())
		m_p2_input_th.join();
}