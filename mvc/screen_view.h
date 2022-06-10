#pragma once
#include <iostream>

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
        std::cout << "\x1B[?25l";
    }

    void cls() override
    {
        std::cout << "\x1B[2J";
    }

    void make_paint(size_t x, size_t y, char c) override
    {
        std::stringstream ss;

        ss << "\x1B[" << y_offset + y << ";" << x_offset + x << "H";
        std::cout << ss.str() << c;
    }
};