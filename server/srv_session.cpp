#include "srv_session.h"
#include "..\game_config.h"

void srv_session::init_gui()
{
	m_gui_model.create_primitive<rectangle>("battlefield",	PPP_MAIN_FIELD_X, PPP_MAIN_FIELD_Y, 
															PPP_MAIN_FIELD_W, PPP_MAIN_FIELD_H);

	m_gui_model.create_primitive<bar>("p1_bar", PPP_M_BAR_X, PPP_M_BAR_Y, PPP_BAR_LEN);
	m_gui_model.create_primitive<bar>("p1_shadow_bar", PPP_S_BAR_X, PPP_S_BAR_Y, PPP_BAR_LEN);

	m_gui_model.create_primitive<bar>("p2_bar", PPP_M_BAR_X, PPP_M_BAR_Y, PPP_BAR_LEN);
	m_gui_model.create_primitive<bar>("p2_shadow_bar", PPP_S_BAR_X, PPP_S_BAR_Y, PPP_BAR_LEN);

	m_gui_model.create_primitive<point>("ball", PPP_M_BALL_X, PPP_M_BALL_Y, 'O');
	m_gui_model.create_primitive<point>("shadow_ball", PPP_S_BALL_X, PPP_S_BALL_Y, 'O');
}

void srv_session::start_game(/*Game options????*/)
{
	net_view_ptr player1_view = make_shared<network_view>(m_p1_client);
	net_view_ptr player2_view = make_shared<network_view>(m_p2_client);

	controller_ptr player1_ctrl = make_shared<controller>(player1_view);
	controller_ptr player2_ctrl = make_shared<controller>(player2_view);

	init_gui();

	//Draw all primitivies
	auto battlefield = m_gui_model.get_primitive("battlefield");

	player1_ctrl->draw(battlefield);
	player2_ctrl->draw(battlefield);

	m_paint_th = std::thread(&srv_session::paint_th, this, player1_ctrl, player2_ctrl);
	m_p1_input_th = std::thread(&srv_session::input_th, this, m_p1_client, player1_ctrl, player2_ctrl, true);
	m_p2_input_th = std::thread(&srv_session::input_th, this, m_p2_client, player2_ctrl, player1_ctrl, false);
}

void srv_session::input_th(		connection_ptr m_client, 
								controller_ptr p1_ctrl,
								controller_ptr p2_ctrl, bool master)
{
	string buffer = "";

	auto main_play_bar = m_gui_model.get_primitive("p1_bar");
	auto shadow_play_bar = m_gui_model.get_primitive("p1_shadow_bar");

	//??????
	if (!master)
	{
		main_play_bar = m_gui_model.get_primitive("p2_bar");
		shadow_play_bar = m_gui_model.get_primitive("p2_shadow_bar");
	}

	auto ball = m_gui_model.get_primitive("ball");
	auto shadow_ball = m_gui_model.get_primitive("shadow_ball");

	int x_step = 0;

	p1_ctrl->draw(main_play_bar);
	p2_ctrl->draw(shadow_play_bar);

	while (true)
	{
		size_t len = m_client->read(buffer);
		
		if (len == 0)
			break;

		char client_input = buffer[0];

		switch(client_input)
		{
			case 'L':
			{
				if (main_play_bar->get_x() > PPP_MAIN_FIELD_X + 1)
					x_step = -1;
				else
					continue;

				break;
			}
			case 'R':
			{
				if (main_play_bar->get_x() < PPP_MAIN_FIELD_W - PPP_BAR_LEN - 1)
					x_step = 1;
				else
					continue;

				break;
			}
			case 'E':
			{
				continue;
			}
			case 'S':
			{
				/*
				if (master)
				{
					m_p1_ready.test_and_set();
					m_p1_ready.notify_one();
				}
				else
				{
					m_p1_ready.test_and_set();
					m_p1_ready.notify_one();
				}
				*/
				continue;
			}
			default:
				continue;
		}

		p1_ctrl->move(main_play_bar, x_step, 0);
		//Mirror shadow primitive moves
		p2_ctrl->move(shadow_play_bar, -x_step, 0);

		//Move ball until game not started
		if (m_state != game_state::start)
		{
			p1_ctrl->move(ball, x_step, 0);
			//Mirror shadow primitive moves
			p2_ctrl->move(shadow_ball, -x_step, 0);
		}
	}

	m_state = game_state::end;
}

void srv_session::paint_th(controller_ptr p1_ctrl, controller_ptr p2_ctrl)
{
	string buffer;

	auto main_ball = m_gui_model.get_primitive("ball");
	auto shadow_ball = m_gui_model.get_primitive("shadow_ball");

	size_t x = PPP_M_BALL_X;
	size_t y = PPP_M_BALL_Y;

	int x_step = 1;
	int y_step = -1;

	p1_ctrl->draw(main_ball);
	p2_ctrl->draw(shadow_ball);

	//Wait for players ready
	//m_p1_ready.wait(false);
	//m_p2_ready.wait(false);

	while (true)
	{
		//Check game status
		if (m_state == game_state::end)
		{
			stop_game();
			break;
		}

		if (x == PPP_MAIN_FIELD_W - 2)
			x_step = -1;

		if (y == PPP_MAIN_FIELD_Y + 2)
			y_step = 1;

		if (x == PPP_MAIN_FIELD_X + 1)
			x_step = 1;

		if (y == PPP_MAIN_FIELD_H - 3)
			y_step = -1;

		x += x_step;
		y += y_step;

		p1_ctrl->move(main_ball, x_step, y_step);
		//Mirror shadow primitive moves
		p2_ctrl->move(shadow_ball, -x_step, -y_step);

		std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(100));
	}
}

void srv_session::stop_game()
{
	m_state = game_state::end;

	/*
	m_p1_ready.test_and_set();
	m_p1_ready.notify_one();
	m_p1_ready.test_and_set();
	m_p1_ready.notify_one();
	*/
	m_p1_client->disconnect();
	m_p2_client->disconnect();
}

void srv_session::wait_end()
{
	if (m_paint_th.joinable())
		m_paint_th.join();
	
	if (m_p1_input_th.joinable())
		m_p1_input_th.join();
	
	if (m_p2_input_th.joinable())
		m_p2_input_th.join();
}