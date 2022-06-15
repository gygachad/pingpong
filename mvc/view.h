#pragma once
#include <list>
#include <vector>
#include <map>
#include <iostream>
#include <utility>
#include <functional>
#include <mutex>

struct char_pixel
{
    size_t m_x;
    size_t m_y;
    char m_c;

    char_pixel(size_t x, size_t y, char c) : m_x(x), m_y(y), m_c(c) {}
};

class view
{
    std::mutex m_screen_lock;
    using paint_map = std::map<std::pair<size_t, size_t>, char>;

    virtual void make_paint(const size_t x, const size_t y, const char c) = 0;
    virtual void make_paint(const std::vector<char_pixel>& paint_char_pixels) = 0;

public:

    view() { }

    virtual void screen_init() = 0;
    virtual void cls() = 0;

    void paint(const paint_map& pixels)
    {
        std::vector<char_pixel> paint_points;
        paint_points.reserve(pixels.size());

        for (const auto& point : pixels)
        {
            size_t x;
            size_t y;
            std::tie(x, y) = point.first;
            char c = point.second;

            paint_points.emplace_back(char_pixel(x, y, c));
        }

        //Call specific handler
        make_paint(paint_points);
    }

    virtual ~view() { }
};