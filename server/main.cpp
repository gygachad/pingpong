#include <iostream>

#include "pingpong_server.h"

using namespace std;

int main(int argc, const char* argv[])
{
	/*
	string cmd_line;
	shared_ptr<ICmd_dispatcher> cmd_dispatched = make_shared<join_server>();

	while (getline(cin, cmd_line))
	{
		if (cmd_line == "exit")
			break;

		//execute
		string answer = cmd_dispatched->execute(cmd_line);

		cout << answer << endl;
	}
	*/
	if (argc < 2)
	{
		std::cout << "Usage: pingpong_server port_num" << std::endl;
		return 0;
	}

	uint16_t port = atoi(argv[1]);

	pingpong_server srv(port);

	srv.set_verbose_out(true);

	srv.start();

	string cmd = "";

	while (getline(cin, cmd))
	{
		if (cmd == "stop")
			break;
	}

	srv.stop();

	return 0;
}
