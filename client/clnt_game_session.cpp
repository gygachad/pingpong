#include <conio.h>

#include "clnt_game_session.h"

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77

void clnt_game_session::start_game(/*Game options????*/)
{
	m_paint_th = std::thread(&clnt_game_session::paint_th, this);
	m_input_th = std::thread(&clnt_game_session::input_th, this);
}

void clnt_game_session::input_th()
{
	string s;

	while (true)
	{
		switch (_getch())
		{
		case KEY_LEFT:
		{
			s = "L\n";
			break;
		}
		case KEY_RIGHT:
		{
			s = "R\n";
			break;
		}
		default:
			continue;
		}

		m_srv->write(s);
	}
}

void clnt_game_session::paint_th()
{
	std::string buffer;
	stringstream ss;
	screen_view scr;

	scr.screen_init();
	scr.cls();
	scr.set_offset(1, 1);

	while (true)
	{
		m_srv->read(buffer);

		std::vector<string> cmd_line = str_tool::split(buffer, "\n");

		for (const auto& cmd : cmd_line)
		{
			std::vector<string> paint_cmd = str_tool::split(cmd, ";");

			if (paint_cmd.size() != 3)
				continue;

			size_t x = 0;
			ss = stringstream(paint_cmd[0]);
			ss >> x;

			size_t y = 0;
			ss = stringstream(paint_cmd[1]);
			ss >> y;

			char c = 0;
			ss = stringstream(paint_cmd[2]);
			ss >> c;

			if (c == 0)
				c = ' ';

			scr.make_paint(x, y, c);
		}
	}
}

void clnt_game_session::wait_end()
{
	if (m_paint_th.joinable())
		m_paint_th.join();

	if (m_input_th.joinable())
		m_input_th.join();
}

void clnt_game_session::stop_game()
{
	m_srv->disconnect();

	if (m_paint_th.joinable())
		m_paint_th.join();

	if (m_input_th.joinable())
		m_input_th.join();
}