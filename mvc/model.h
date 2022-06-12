#pragma once

#include <iostream>
#include <sstream>
#include <list>
#include <vector>
#include <mutex>
#include <map>

#include "view.h"

struct char_pixel
{
    size_t m_x;
    size_t m_y;
    char m_c;

    char_pixel(size_t x, size_t y, char c) : m_x(x), m_y(y), m_c(c) {}
};

class IGui_primitive
{

protected:
    size_t m_x = 0;
    size_t m_y = 0;
    size_t m_w = 0;
    size_t m_h = 0;

public:
    virtual void move(size_t x, size_t y) = 0;
    virtual void get_draw_data(view::paint_map& pixels) = 0;
    virtual void clean(view::paint_map& pixels) = 0;

    virtual ~IGui_primitive() {}

    size_t get_x() { return m_x; }
    size_t get_y() { return m_y; }
    size_t get_w() { return m_w; }
    size_t get_h() { return m_h; }
};


class point : public IGui_primitive
{
    char m_c;

public:
    point(size_t x = 0, size_t y = 0, char c = ' ') : m_c(c)
    {
        m_x = x;
        m_y = y;
        m_w = 1;
        m_h = 1;
    }

    void move(size_t x, size_t y) override
    {
        m_x += x;
        m_y += y;
    }
    
    void get_draw_data(view::paint_map& pixels) override
    {
        auto point = std::make_pair(m_x, m_y);
        pixels[point] = m_c;
    }
    
    void clean(view::paint_map& pixels) override
    { 
        auto point = std::make_pair(m_x, m_y);
        pixels[point] = ' ';
    }
};

class rectangle : public IGui_primitive
{
    //map<>
    std::vector<point> m_mainfield;

public:
    rectangle(size_t x, size_t y, size_t w, size_t h)
    {
        m_x = x;
        m_y = y;
        m_w = w;
        m_h = h;

        m_mainfield.reserve(2 * m_w + 2 * m_h);

        m_mainfield.emplace_back(point(0, 0, '\xDA'));
        m_mainfield.emplace_back(point(m_w - 1, 0, '\xBF'));
        m_mainfield.emplace_back(point(m_w - 1, m_h - 1, '\xD9'));
        m_mainfield.emplace_back(point(0, m_h - 1, '\xC0'));

        //Draw horizontal
        for (size_t i = 1; i < m_w - 1; i++)
        {
            m_mainfield.emplace_back(point(i, 0, '\xC4'));
            m_mainfield.emplace_back(point(i, m_h - 1, '\xC4'));
        }

        //Draw vertical
        for (size_t i = 1; i < m_h - 1; i++)
        {
            m_mainfield.emplace_back(point(0, i, '\xB3'));
            m_mainfield.emplace_back(point(m_w - 1, i, '\xB3'));
        }
    }

    void move(size_t x, size_t y) override
    {
        m_x += x;
        m_y += y;

        for (auto& cp : m_mainfield)
            cp.move(x, y);
    }
    
    void get_draw_data(view::paint_map& pixels) override
    {
        for (auto& cp : m_mainfield)
            cp.get_draw_data(pixels);
    }
    
    void clean(view::paint_map& pixels) override
    {
        for (auto& cp : m_mainfield)
            cp.clean(pixels);
    }
};

class bar : public IGui_primitive
{
    std::vector<point> m_mainfield;

public:
    bar(size_t x, size_t y, size_t len, char c = '=')
    {
        m_x = x;
        m_y = y;
        m_w = len;
        m_h = 1;

        m_mainfield.reserve(len);

        for (size_t i = 0; i < len; i++)
        {
            m_mainfield.emplace_back(point(m_x + i, m_y, c));
        }
    }

    void move(size_t x, size_t y) override
    {
        m_x += x;
        m_y += y;

        for (auto& cp : m_mainfield)
            cp.move(x, y);
    }
    
    void get_draw_data(view::paint_map& pixels) override
    {
        //Draw picture on new position
        for (auto& cp : m_mainfield)
            cp.get_draw_data(pixels);
    }

    void clean(view::paint_map& pixels) override
    {
        for (auto& cp : m_mainfield)
            cp.clean(pixels);
    }

    size_t get_x() { return m_x; }
    size_t get_y() { return m_y; }
    size_t get_w() { return m_w; }
    size_t get_h() { return m_h; }
};

class model
{
    using primitive_ptr = std::shared_ptr<IGui_primitive>;
    std::map<std::string, primitive_ptr> gui_primitives;

public:
    template<typename T, typename... Args>
    bool create_primitive(const std::string& name, Args... arguments)
    {
        if (gui_primitives.contains(name))
            return false;

        gui_primitives[name] = std::make_shared<T>(T(std::forward<Args>(arguments)...));

        return true;
    }

    auto get_primitive(const std::string& name)
    {
        return gui_primitives[name];
    }
};