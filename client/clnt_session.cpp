#include <conio.h>
#include <map>
#include <sstream>

#include "..\mvc\screen_view.h"
#include "..\str_tool.h"
#include "clnt_session.h"

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define SPACEBAR 32

void clnt_session::start_game()
{
	m_paint_th = std::thread(&clnt_session::paint_th, this);
	m_input_th = std::thread(&clnt_session::input_th, this);
}

void clnt_session::input_th()
{
	std::map<int, std::string> m_keymap = {	{KEY_LEFT , "L\n"}, 
											{KEY_RIGHT, "R\n"}, 
											{SPACEBAR, "S\n"}};

	std::stringstream ss;

	while (true)
	{
		int c = _getch();
		ss << c << "\n";
		
		size_t len = m_srv.write(ss.str());

		if (len == 0)
			break;

		//Clear string stream
		ss.str("");
	}
}

void clnt_session::paint_th()
{
	std::string buffer;
	std::stringstream ss;
	screen_view scr;

	scr.screen_init();
	scr.cls();
	scr.set_offset(1, 1);
	
	std::vector<char_pixel> paint_pixel;

	while (true)
	{
		size_t len = m_srv.read(buffer);

		if (len == 0)
			break;

		paint_pixel.clear();

		std::vector<std::string> cmd_line = str_tool::split(buffer, "\n");

		for (const auto& cmd : cmd_line)
		{
			std::vector<std::string> paint_cmd = str_tool::split(cmd, ";");

			if (paint_cmd.size() != 3)
				continue;

			size_t x = 0;
			ss = std::stringstream(paint_cmd[0]);
			ss >> x;

			size_t y = 0;
			ss = std::stringstream(paint_cmd[1]);
			ss >> y;

			char c = 0;
			ss = std::stringstream(paint_cmd[2]);
			ss >> c;

			if (c == 0)
				c = ' ';

			paint_pixel.emplace_back(char_pixel(x, y, c));
		}

		scr.make_paint(paint_pixel);
	}
}

void clnt_session::wait_end()
{
	if (m_paint_th.joinable())
		m_paint_th.join();

	if (m_input_th.joinable())
		m_input_th.join();
}

void clnt_session::stop_game()
{
	m_srv.disconnect();

	if (m_paint_th.joinable())
		m_paint_th.join();

	if (m_input_th.joinable())
		m_input_th.join();
}