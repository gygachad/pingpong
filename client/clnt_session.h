#pragma once

#include <sstream>
#include <thread>

#include "../connection.h"

class clnt_session
{
	connection& m_srv;

	std::thread m_paint_th;
	std::thread m_input_th;

public:
	clnt_session(connection& srv) : m_srv(srv) {	}

	void start_game(/*Game options????*/);
	
	void paint_th();
	void input_th();

	void wait_end();
	void stop_game();
};