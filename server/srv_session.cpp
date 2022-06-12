#include "srv_session.h"
#include "..\game_config.h"

void srv_session::start_game(/*Game options????*/)
{
	net_view_ptr player1_view = make_shared<network_view>(m_p1_client);
	net_view_ptr player2_view = make_shared<network_view>(m_p2_client);

	controller_ptr player1_ctrl = make_shared<controller>(player1_view);
	controller_ptr player2_ctrl = make_shared<controller>(player2_view);

	auto battlefield = m_main_mdl.create_primitive<rectangle>(PPP_MAIN_FIELD_X, PPP_MAIN_FIELD_Y, PPP_MAIN_FIELD_W, PPP_MAIN_FIELD_H);

	//Draw all primitivies
	player1_ctrl->draw(battlefield);
	player2_ctrl->draw(battlefield);

	m_paint_th = std::thread(&srv_session::paint_th, this, player1_ctrl, player2_ctrl);
	m_p1_input_th = std::thread(&srv_session::input_th, this, m_p1_client, player1_ctrl, player2_ctrl);
	m_p2_input_th = std::thread(&srv_session::input_th, this, m_p2_client, player2_ctrl, player1_ctrl);
}

void srv_session::input_th(		connection_ptr m_client, 
								controller_ptr p1_ctrl,
								controller_ptr p2_ctrl)
{

	string buffer = "";

	//mutex???
	auto main_play_bar = m_main_mdl.create_primitive<bar>(PPP_M_BAR_X, PPP_M_BAR_Y, PPP_BAR_LEN);
	auto shadow_play_bar = m_main_mdl.create_primitive<bar>(PPP_S_BAR_X, PPP_S_BAR_Y, PPP_BAR_LEN);

	int x_step = 0;

	p1_ctrl->draw(main_play_bar);
	p2_ctrl->draw(shadow_play_bar);

	while (true)
	{
		size_t len = m_client->read(buffer);
		
		if (len == 0)
			break;

		if (buffer == "L\n")
		{
			if (main_play_bar->get_x() > PPP_MAIN_FIELD_X + 1)
				x_step = -1;
			else
				continue;
			
		}
		else if (buffer == "R\n")
		{
			if (main_play_bar->get_x() < PPP_MAIN_FIELD_W - PPP_BAR_LEN - 1)
				x_step = 1;
			else
				continue;
		}
		else if (buffer == "S\n")
		{
			m_state = game_state::start;
			//condition variable
		}
		else if (buffer == "S\n")
		{
			m_state = game_state::end;
		}
		else
			continue;

		p1_ctrl->move(main_play_bar, x_step, 0);
		//Mirror shadow primitive moves
		p2_ctrl->move(shadow_play_bar, -x_step, 0);
	}

	m_state = game_state::end;
}

void srv_session::paint_th(controller_ptr p1_ctrl, controller_ptr p2_ctrl)
{
	string buffer;

	auto main_ball = m_main_mdl.create_primitive<point>(PPP_M_BALL_X, PPP_M_BALL_Y, 'O');
	auto shadow_ball = m_main_mdl.create_primitive<point>(PPP_S_BALL_X, PPP_S_BALL_Y, 'O');

	size_t x = PPP_M_BALL_X;
	size_t y = PPP_M_BALL_Y;

	int x_step = 1;
	int y_step = -1;

	p1_ctrl->draw(main_ball);
	p2_ctrl->draw(shadow_ball);

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