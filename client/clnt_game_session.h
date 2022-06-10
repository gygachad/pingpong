#pragma once

#include <sstream>
#include <thread>

#include "..\mvc\controller.h"
#include "..\mvc\screen_view.h"
#include "..\str_tool.h"
#include "..\pingpong_client.h"
#include "..\game_proto.h"

class clnt_game_session
{
	using client_ptr = std::shared_ptr<pingpong_client>;
	client_ptr m_srv;

	std::thread m_paint_th;
	std::thread m_input_th;

public:
	clnt_game_session(client_ptr srv) : m_srv(srv) {	}

	void start_game(/*Game options????*/);
	
	void paint_th();
	void input_th();

	void wait_end();
	void stop_game();
};