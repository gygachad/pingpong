#pragma once
#include <list>
#include <vector>
#include <map>
#include <iostream>
#include <utility>
#include <functional>
#include <mutex>

class view
{
    std::mutex m_screen_lock;
    std::map<std::pair<size_t, size_t>, char> m_screen_snapshot;

    virtual void make_paint(size_t x, size_t y, char c) = 0;

public:
    using paint_map = std::map<std::pair<size_t, size_t>, char>;

    view() { }

    virtual void screen_init() = 0;
    virtual void cls() = 0;

    void paint(const paint_map& pixels)
    {
        {
            //make shared lock??
            std::lock_guard<std::mutex> lock(m_screen_lock);

            for(const auto& point : pixels)
            {
                if (m_screen_snapshot.contains(point.first))
                {
                    if (m_screen_snapshot[point.first] == point.second)
                        continue;
                }

                size_t x;
                size_t y;
                std::tie(x, y) = point.first;

                m_screen_snapshot[point.first] = point.second;

                //Call specific handler
                make_paint(x, y, point.second);
            }
        }
    }

    virtual ~view() { }
};