#pragma once
#include <iostream>
#include <vector>
#include <mutex>

#include "view.h"

class screen_view : public view
{
    size_t x_offset = 0;
    size_t y_offset = 0;

public:
    void set_offset(size_t x, size_t y)
    {
        x_offset = x;
        y_offset = y;
    }

    void screen_init() override
    {
        std::cout << "\x1B[?25l";//make cursor invisible
        std::cout << "\x1B[28m";//set hidden/invisible mode
    }

    void cls() override
    {
        std::cout << "\x1B[2J";
    }

    void make_paint(const size_t x, const size_t y, const char c) override
    {
        std::stringstream ss;
        ss << "\x1B[" << y_offset + y << ";" << x_offset + x << "H";
        std::cout << ss.str() << c;
    }

    void make_paint(const std::vector<char_pixel>& char_pixels) override
    {
        std::stringstream ss;
        for (const char_pixel& p : char_pixels)
        {
            ss << "\x1B[" << y_offset + p.m_y << ";" << x_offset + p.m_x << "H";
            std::cout << ss.str() << p.m_c;
        }
    }
};