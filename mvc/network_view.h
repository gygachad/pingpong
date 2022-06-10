#pragma once
#include <iostream>

#include "view.h"

class network_view : public view
{
    pingpong_client::client_ptr m_client;

public:
    network_view(pingpong_client::client_ptr client) : m_client(client)
    {

    }

    void screen_init() override {}
    void cls() override {}

    void make_paint(size_t x, size_t y, char c) override
    {
        //Serialize data and send to client
        std::stringstream ss;

        //Send binary data - not string
        ss << x << ";" << y << ";" << c << "\n";
        m_client->write(ss.str());
    }
};