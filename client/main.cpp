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
#include "..\mvc\controller.h"
#include "..\mvc\screen_view.h"
#include "..\connection.h"

#include "clnt_session.h"

std::mutex g_screen_lock;

void move_ball_th(controller& ctrl)
{
    model mdl;
    auto battlefield = mdl.create_primitive<rectangle>(0, 0, 20, 25);
    auto ball = mdl.create_primitive<point>(5, 5, '*');

    size_t x = 5;
    size_t y = 5;

    int x_step = 1;
    int y_step = 1;

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
        /*
        ball->clean();
        ball->move(x - ball->get_x(), y - ball->get_y());
        ball->draw();
        */
    }
}

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77

void get_button_th(controller& ctrl)
{
    size_t x = 1;

    model mdl;

    auto space_bar = mdl.create_primitive<bar>(1, 23, 4);

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
        
        /*
        space_bar->clean();
        space_bar->move(x - space_bar->get_x(), 0);
        space_bar->draw();
        */
    }
}

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

    std::shared_ptr<connection> clnt = make_shared<connection>();

    clnt->connect(ip, port);    
    
    clnt_session session(clnt);

    session.start_game();
    session.wait_end();
}