#pragma once
#include <iostream>
#include <vector>

#include "../connection.h"

#include "view.h"

class network_view : public view
{
    using connection_ptr = std::shared_ptr<connection>;
    connection_ptr m_client;

public:
    network_view(connection_ptr client) : m_client(client) {}
    ~network_view() {}

    void operator=(const network_view& other)
    {
        m_client = other.m_client;
    }

    void screen_init() override {}
    void cls() override {}

    void make_paint(const size_t x, const size_t y, const char c) override
    {
        std::stringstream ss;
        ss << x << ";" << y << ";" <<c << "\n";
        //TODO: Send binary data - not string
        m_client->write(ss.str());
    }

    void make_paint(const std::vector<char_pixel>& char_pixels) override
    {
        /*
        std::stringstream ss;

        for (const auto& p : char_pixels)
            ss << p.m_x << ";" << p.m_y << ";" << p.m_c << "\n";
            
        m_client->write(ss.str());
        */

        //Serialize data and send to client
        unsigned long* coded = new unsigned long[char_pixels.size() * sizeof(unsigned long)];

        size_t i = 0;
        for (const auto& p : char_pixels)
        {
            coded[i] = 0;
            coded[i] |= p.m_c << 24;
            coded[i] |= (p.m_y & 0xFFF) << 12;
            coded[i] |= (p.m_x & 0xFFF);
            i++;
        }

        m_client->write(coded, sizeof(coded) * char_pixels.size());

        delete[] coded;
    }
};