// xo_client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <sstream>
#include <list>
#include <array>
#include <vector>
#include <thread>
#include <chrono>
#include <map>

#include "../mvc/model.h"
#include "../mvc/screen_view.h"
#include "../connection.h"

#include "clnt_session.h"

#ifdef SET_WIN_CONSOLE_MODE
void set_console_mode()
{
    HANDLE hInCon = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hOutCon = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD dwInOrigMode = 0;
    DWORD dwOutOrigMode = 0;

    GetConsoleMode(hInCon, &dwInOrigMode);
    GetConsoleMode(hOutCon, &dwOutOrigMode);

    DWORD ReqOutMode = ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
    DWORD ReqInMode = ENABLE_VIRTUAL_TERMINAL_INPUT;

    DWORD OutMode = dwOutOrigMode | ReqOutMode;
    SetConsoleMode(hOutCon, OutMode);

    DWORD InMode = dwInOrigMode | ReqInMode;
    SetConsoleMode(hInCon, InMode);
}
#endif

int main(int argc, const char* argv[])
{

#ifdef SET_WIN_CONSOLE_MODE
    set_console_mode();
#endif

    if (argc < 3)
    {
        std::cout << "Usage: pingpong_client server_addr port_num" << std::endl;
        return 0;
    }
    std::stringstream ss(argv[2]);

    uint16_t port = 0;
    ss >> port;

    std::string ip = std::string(argv[1]);

    asio::io_service io_srv;

    connection clnt(io_srv);

    if (!clnt.connect(ip, port))
        return 0;
    
    clnt_session session(clnt);

    session.start_game();
    session.wait_end();
}