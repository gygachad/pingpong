#include "srv_session.h"

#define PPP_MAIN_FIELD_X 0
#define PPP_MAIN_FIELD_Y 0
#define PPP_MAIN_FIELD_W 20
#define PPP_MAIN_FIELD_H 15

#define PPP_M_BAR_X 1
#define PPP_M_BAR_Y 18
#define PPP_S_BAR_X 1
#define PPP_S_BAR_Y 1
#define PPP_BAR_LEN 5

#define PPP_M_BALL_X 3
#define PPP_M_BALL_Y 17
#define PPP_S_BALL_X 3
#define PPP_S_BALL_Y 2

void srv_session::start_game(/*Game options????*/)
{
	net_view_ptr player1_view = make_shared<network_view>(m_p1_client);
	net_view_ptr player2_view = make_shared<network_view>(m_p2_client);

	controller_ptr player1_ctrl = make_shared<controller>(player1_view);
	controller_ptr player2_ctrl = make_shared<controller>(player2_view);

	auto battlefield = m_main_mdl.create_primitive<rectangle>(PPP_MAIN_FIELD_X, PPP_MAIN_FIELD_Y, PPP_MAIN_FIELD_H, PPP_MAIN_FIELD_W);

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

	p1_ctrl->draw(main_play_bar);
	p2_ctrl->draw(shadow_play_bar);

	while (true)
	{
		size_t len = m_client->read(buffer);

		if (buffer == "L\n")
		{
			if (main_play_bar->get_x() > PPP_MAIN_FIELD_X + 1)
			{
				p1_ctrl->move(main_play_bar, -1, 0);
				p2_ctrl->move(shadow_play_bar, -1, 0);
			}
		}
		else if (buffer == "R\n")
		{
			if (main_play_bar->get_x() < PPP_MAIN_FIELD_H - PPP_BAR_LEN - 1)
			{
				p1_ctrl->move(main_play_bar, 1, 0);
				p2_ctrl->move(shadow_play_bar, 1, 0);
			}
		}
		else
			continue;
	}
}

void srv_session::paint_th(controller_ptr p1_ctrl, controller_ptr p2_ctrl)
{
	string buffer;

	auto main_ball = m_main_mdl.create_primitive<point>(PPP_M_BALL_X, PPP_M_BALL_Y, '*');
	auto shadow_ball = m_main_mdl.create_primitive<point>(PPP_S_BALL_X, PPP_S_BALL_Y, '*');

	size_t x = 5;
	size_t y = 5;

	int x_step = 1;
	int y_step = 1;

	p1_ctrl->draw(main_ball);
	p2_ctrl->draw(shadow_ball);

	while (true)
	{
		if (x == PPP_MAIN_FIELD_H - 2)
			x_step = -1;

		if (y == PPP_MAIN_FIELD_W - 3)
			y_step = -1;

		if (x == PPP_MAIN_FIELD_X + 1)
			x_step = 1;

		if (y == PPP_MAIN_FIELD_Y + 2)
			y_step = 1;

		x += x_step;
		y += y_step;

		p1_ctrl->move(main_ball, x - main_ball->get_x(), y - main_ball->get_y());
		p2_ctrl->move(shadow_ball, x - shadow_ball->get_x(), y - shadow_ball->get_y());

		std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(100));
	}
}

void srv_session::stop_game()
{
	m_p1_client->disconnect();
	m_p2_client->disconnect();

	if (m_paint_th.joinable())
		m_paint_th.join();

	if (m_p1_input_th.joinable())
		m_p1_input_th.join();

	if (m_p2_input_th.joinable())
		m_p2_input_th.join();
}