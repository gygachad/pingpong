#ifdef SET_WIN_CONSOLE_MODE
#include <conio.h>
#endif
#include <map>
#include <sstream>

#include "../mvc/screen_view.h"
#include "../str_tool.h"
#include "clnt_session.h"

void clnt_session::start_game()
{
	m_paint_th = std::thread(&clnt_session::paint_th, this);
	m_input_th = std::thread(&clnt_session::input_th, this);
}

void clnt_session::input_th()
{
	std::stringstream ss;

	while (true)
	{
#ifdef SET_WIN_CONSOLE_MODE
		int c = _getch();
#else
		int c = getc(stdin);
#endif
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

	while (true)
	{
		unsigned long code = 0;

		size_t len = m_srv.read(&code, sizeof(unsigned long));

		if (len == 0)
			break;

		if (len != 4)
			continue;

		char c = code >> 24;
		size_t y = (code >> 12) & 0xFFF;
		size_t x = code & 0xFFF;

		scr.make_paint(x, y, c);
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