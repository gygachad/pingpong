#include <iostream>
#include <sstream>

#include "server.h"

using namespace std;

int main(int argc, const char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: pingpong_server port_num" << std::endl;
		return 0;
	}

	std::stringstream ss(argv[1]);

	uint16_t port = 0;
	ss >> port;

	asio::io_service io_srv;

	server srv(io_srv, port);

	srv.set_verbose_out(true);
	srv.start();

	string cmd = "";
	
	while (getline(cin, cmd))
	{
		if (cmd == "stop")
			break;
	}

	srv.stop();

	std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(1000));

	return 0;
}
