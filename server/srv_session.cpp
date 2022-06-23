#include <ctime>
#include <climits>

#include "../game_config.h"
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
	//Initial state
	m_state.store(game_state::wait);

	m_paint_th = std::thread(&srv_session::paint_th, this, m_player1, m_player2);
	m_p1_input_th = std::thread(&srv_session::input_th, this, m_player1, m_player2, true);
	m_p2_input_th = std::thread(&srv_session::input_th, this, m_player2, m_player1, false);
}

void srv_session::input_th(player_ptr player1, player_ptr player2, bool master)
{
	std::string buffer;
	std::string custom_player_name = "";

	auto player_connection = player1->get_connection();

	player1->change_player_state(player_state::wait);
	player1->change_shadow_player_state(player_state::wait);

	size_t battlefield_x = player1->get_battlefield_x();
	size_t battlefield_w = player2->get_battlefield_w();

	int x_step = 0;

	size_t bar_len = player1->get_bar_len();

	std::unique_lock<std::mutex> lock(m_bb_lock, std::defer_lock);

	//Mode player2 ball to opposite side
	if (master)
		player2->move_ball(SHADOW_BALL_OFFSET_X, SHADOW_BALL_OFFSET_Y);

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
				if (player1->get_bar_pos() > battlefield_x + 1)
					x_step = -1;
				else
					continue;
				break;
			}
			case KEY_RIGHT:
			{
				if (player1->get_bar_pos() < battlefield_w - bar_len - 1)
					x_step = 1;
				else
					continue;
				break;
			}
			case KEY_SPACEBAR:
			{
				if (player1->m_state.load() == player_state::wait)
				{
					player2->change_shadow_player_state(player_state::ready);
					player1->change_player_state(player_state::ready);
				}
				continue;
			}
			default:
			{
				if (player1->m_state.load() == player_state::wait)
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

			//Loack Bar/Ball operation
			lock.lock();
			size_t y = player1->get_ball_y();
			if (y != MAIN_FIELD_H - 2)
			{
				player1->move_bar(x_step);
				//Mirror shadow primitive moves
				player2->move_shadow_bar(-x_step);
			}
			lock.unlock();

			if (master)
			{
				//Move ball until game not started
				if (player1->m_state.load() != player_state::start)
				{
					player1->move_ball(x_step, 0);
					player2->move_ball(-x_step, 0);
				}
			}
		}
	}
}

void srv_session::paint_th(player_ptr player1, player_ptr player2)
{
	std::string buffer;

	std::srand(std::time(0));
	int x_step = (std::rand() % 2) ? 1 : -1;
	int y_step = -1;

	//Wait for players ready
	player1->wait_for_ready();
	player2->wait_for_ready();

	//Start game here
	m_state.store(game_state::start);
	//Notify all beacuse wait (stop_game) called from 2 input_th
	m_start_game.test_and_set();
	m_start_game.notify_all();

	for (size_t i = 300; i; i--)
	{
		if (m_state.load() == game_state::stop)
			return;

		player1->set_ready_timer(i);
		player2->set_ready_timer(i);
		std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(1));
	}

	player1->clean_ready_timer();
	player2->clean_ready_timer();

	player1->change_player_state(player_state::start);
	player2->change_player_state(player_state::start);
	player1->change_shadow_player_state(player_state::start);
	player2->change_shadow_player_state(player_state::start);

	size_t x = player1->get_ball_x();
	size_t y = player1->get_ball_y();
	size_t shadow_x = player2->get_ball_x();
	size_t shadow_y = player2->get_ball_y();

	size_t bar_x = 0;
	size_t p1_bar_w = player2->get_bar_len();
	size_t p2_bar_w = player1->get_bar_len();

	std::unique_lock<std::mutex> lock(m_bb_lock, std::defer_lock);

	while (true)
	{
		//Check game status
		if (m_state.load() == game_state::stop)
			break;

		m_bb_lock.lock();

		if (x_step < 0)
		{
			if (x - MAIN_FIELD_X - 1 < std::abs(x_step))
			{
				if (x != MAIN_FIELD_X + 1)
					x_step = int(MAIN_FIELD_X - x + 1);
				else
					x_step = 1;
			}
		}
		else
		{
			if (MAIN_FIELD_W - x - 2 < x_step)
			{
				if (x != MAIN_FIELD_W - 2)
					x_step = int(MAIN_FIELD_W - x - 2);
				else
					x_step = -1;
			}
		}

		//Change ball angle if ball hit bar corners
		if (shadow_y == MAIN_FIELD_H - 3)
		{
			bar_x = player2->get_bar_pos();

			if (shadow_x - x_step >= bar_x && shadow_x - x_step < bar_x + p2_bar_w)
			{
				if (y_step != 1)
				{
					y_step = 1;

					if ((bar_x + 1 == shadow_x - x_step) || (bar_x + p2_bar_w - 2 == shadow_x - x_step))
						x_step = x_step * 2;
					if ((bar_x == shadow_x - x_step) || (bar_x + p2_bar_w - 1 == shadow_x - x_step))
						x_step = x_step * 3;
				}
			}
		}

		if (y == MAIN_FIELD_H - 3)
		{
			bar_x = player1->get_bar_pos();

			if (x + x_step >= bar_x && x + x_step < bar_x + p1_bar_w)
			{
				if (y_step != -1)
				{
					y_step = -1;

					if ((bar_x + 1 == x + x_step) || (bar_x + p1_bar_w - 2 == x + x_step))
						x_step = x_step * 2;
					if ((bar_x == x + x_step) || (bar_x + p1_bar_w - 1 == x + x_step))
						x_step = x_step * 3;
				}
			}
		}

		//Check for step correction again
		if (x_step < 0)
		{
			if (x - MAIN_FIELD_X - 1 < std::abs(x_step))
			{
				if (x != MAIN_FIELD_X + 1)
					x_step = int(MAIN_FIELD_X - x + 1);
				else
					x_step = 1;
			}
		}
		else
		{
			if (MAIN_FIELD_W - x - 2 < x_step)
			{
				if (x != MAIN_FIELD_W - 2)
					x_step = int(MAIN_FIELD_W - x - 2);
				else
					x_step = -1;
			}
		}

		if (shadow_y == MAIN_FIELD_H - 2)
		{
			y_step = 1;
			player2->add_goal();
			player1->add_shadow_goal();

		}
		if (y == MAIN_FIELD_H - 2)
		{
			y_step = -1;
			player1->add_goal();
			player2->add_shadow_goal();
		}

		x += x_step;
		y += y_step;
		shadow_x -= x_step;
		shadow_y -= y_step;

		player1->move_ball(x_step, y_step);
		player2->move_ball(-x_step, -y_step);

		m_bb_lock.unlock();

		std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(SRV_TRESHOLD));
	}
}

void srv_session::stop_game()
{
	if (m_state.load() == game_state::stop)
		return;

	m_player1->change_player_state(player_state::stop);
	m_player2->change_player_state(player_state::stop);

	//Wait until game state changed
	m_start_game.wait(false);

	m_state.store(game_state::stop);
	m_stop_game.test_and_set();
	m_stop_game.notify_one();
}

void srv_session::wait_end()
{
	m_stop_game.wait(false);

	m_p1_client->disconnect();
	m_p2_client->disconnect();

	m_paint_th.join();
	m_p1_input_th.join();
	m_p2_input_th.join();
}