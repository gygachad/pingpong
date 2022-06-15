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
#include <conio.h>

#include "..\mvc\model.h"
#include "..\mvc\screen_view.h"
#include "..\connection.h"

#include "clnt_session.h"

/*
void move_ball_th(controller& ctrl)
{
    model mdl;
    mdl.create_primitive<rectangle>("battlefield", 0, 0, 20, 25);
    mdl.create_primitive<point>("ball", 5, 5, '*');

    size_t x = 5;
    size_t y = 5;

    int x_step = 1;
    int y_step = 1;

    auto battlefield = mdl.get_primitive("battlefield");
    auto ball = mdl.get_primitive("ball");

    ctrl.draw(battlefield);
    ctrl.draw(ball);

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(50));

        if (x == battlefield->get_w() - 2)
            x_step = -1;

        if (y == battlefield->get_h() - 3)
            y_step = -1;

        if (x == battlefield->get_x() + 1)
            x_step = 1;

        if (y == battlefield->get_y() + 1)
            y_step = 1;

        x += x_step;
        y += y_step;

        ctrl.move(ball, x - ball->get_x(), y - ball->get_y());
    }
}

void get_button_th(controller& ctrl)
{
    size_t x = 1;

    model mdl;

    mdl.create_primitive<bar>("space_bar", 1, 23, 4);

    auto space_bar = mdl.get_primitive("space_bar");

    ctrl.draw(space_bar);

    while (1)
    {
        switch (_getch())
        {
        case KEY_LEFT:
        {
            if (space_bar->get_x() > 1)
                x -= 1;
            else
                continue;
            break;
        }
        case KEY_RIGHT:
        {
            if (space_bar->get_x() < 15)
                x += 1;
            else
                continue;
            break;
        }
        default:
            continue;
        }

        ctrl.move(space_bar, x - space_bar->get_x(), 0);
    }
}
*/

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

int main(int argc, const char* argv[])
{
    set_console_mode();

    /*
    for (char i = 0; i < 255; i++)
    {
        cout << i;
    }
    */
    /*
    shared_ptr<screen_view> scr_view = make_shared<screen_view>();

    scr_view->screen_init();
    scr_view->cls();
    scr_view->set_offset(1, 1);

    controller ctrl(scr_view);

    std::thread draw_th = std::thread(move_ball_th, std::ref(ctrl));
    std::thread button_th = std::thread(get_button_th, std::ref(ctrl));

    draw_th.join();
    button_th.join();
    */

    if (argc < 3)
    {
        std::cout << "Usage: pingpong_client server_addr port_num" << std::endl;
        return 0;
    }

    std::string ip = std::string(argv[1]);
    uint16_t port = atoi(argv[2]);

    asio::io_service io_srv;

    connection clnt(io_srv);

    if (!clnt.connect(ip, port))
        return 0;
    
    clnt_session session(clnt);

    session.start_game();
    session.wait_end();
}