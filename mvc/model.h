#pragma once

#include <iostream>
#include <sstream>
#include <list>
#include <vector>
#include <mutex>
#include <map>
#include <shared_mutex>

#include "view.h"

class IGui_primitive
{
protected:
    using paint_map = std::map<std::pair<size_t, size_t>, char>;

    size_t m_x = 0;
    size_t m_y = 0;
    size_t m_w = 0;
    size_t m_h = 0;

public:
    const static char empty_char = ' ';
    virtual void move(size_t x, size_t y) = 0;
    virtual void get_data(paint_map& pixels) = 0;
    virtual void clean(paint_map& pixels) = 0;

    virtual ~IGui_primitive() {}

    size_t get_x() { return m_x; }
    size_t get_y() { return m_y; }
    size_t get_w() { return m_w; }
    size_t get_h() { return m_h; }

    void set_x(size_t x) { m_x = x; }
    void set_y(size_t y) { m_y = y; }
    void set_w(size_t w) { m_w = w; }
    void set_h(size_t h) { m_h = h; }
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
    
    void get_data(paint_map& pixels) override
    {
        auto point = std::make_pair(m_x, m_y);
        pixels[point] = m_c;
    }
    
    void clean(paint_map& pixels) override
    { 
        auto point = std::make_pair(m_x, m_y);
        pixels[point] = ' ';
    }
};

class rectangle : public IGui_primitive
{
    std::vector<point> m_mainfield;

public:
    rectangle(size_t x = 0, size_t y = 0, size_t w = 1, size_t h = 1)
    {
        m_x = x;
        m_y = y;
        m_w = w;
        m_h = h;

        m_mainfield.reserve(2 * m_w + 2 * m_h);

        //Left up corner
        m_mainfield.emplace_back(point(m_x, m_y, '\xDA'));
        //Right up corner
        m_mainfield.emplace_back(point(m_x + m_w - 1, m_y, '\xBF'));
        //Right bottom corner
        m_mainfield.emplace_back(point(m_x + m_w - 1, m_y + m_h - 1, '\xD9'));
        //Left bottom corner
        m_mainfield.emplace_back(point(m_x, m_y + m_h - 1, '\xC0'));

        //Horizontal lines
        for (size_t i = 1; i < m_w - 1; i++)
        {
            m_mainfield.emplace_back(point(m_x + i, m_y, '\xC4'));
            m_mainfield.emplace_back(point(m_x + i, m_y + m_h - 1, '\xC4'));
        }

        //Vertical
        for (size_t i = 1; i < m_h - 1; i++)
        {
            m_mainfield.emplace_back(point(m_x, m_y + i, '\xB3'));
            m_mainfield.emplace_back(point(m_x + m_w - 1, m_y + i, '\xB3'));
        }
    }

    void move(size_t x, size_t y) override
    {
        m_x += x;
        m_y += y;

        for (auto& cp : m_mainfield)
            cp.move(x, y);
    }
    
    void get_data(paint_map& pixels) override
    {
        for (auto& cp : m_mainfield)
            cp.get_data(pixels);
    }
    
    void clean(paint_map& pixels) override
    {
        for (auto& cp : m_mainfield)
            cp.clean(pixels);
    }
};

class line : public IGui_primitive
{
    std::vector<point> m_mainfield;

public:
    line(size_t x = 0, size_t y = 0, size_t len = 1, char c = '\xC4')
    {
        m_x = x;
        m_y = y;
        m_w = len;
        m_h = 1;

        m_mainfield.reserve(len);

        for (size_t i = 0; i < len; i++)
            m_mainfield.emplace_back(point(m_x + i, m_y, c));
    }

    void move(size_t x, size_t y) override
    {
        m_x += x;
        m_y += y;

        for (auto& cp : m_mainfield)
            cp.move(x, y);
    }
    
    void get_data(paint_map& pixels) override
    {
        //Draw picture on new position
        for (auto& cp : m_mainfield)
            cp.get_data(pixels);
    }

    void clean(paint_map& pixels) override
    {
        for (auto& cp : m_mainfield)
            cp.clean(pixels);
    }
};

class text : public IGui_primitive
{
    std::vector<point> m_mainfield;

public:
    text(size_t x = 0, size_t y = 0, const std::string& text = "")
    {
        m_x = x;
        m_y = y;
        m_w = text.length();
        m_h = 1;

        m_mainfield.reserve(m_w);

        for (size_t i = 0; i < m_w; i++)
            m_mainfield.emplace_back(point(m_x + i, m_y, text[i]));
    }

    void move(size_t x, size_t y) override
    {
        m_x += x;
        m_y += y;

        for (auto& cp : m_mainfield)
            cp.move(x, y);
    }

    void get_data(paint_map& pixels) override
    {
        //Draw picture on new position
        for (auto& cp : m_mainfield)
            cp.get_data(pixels);
    }

    void clean(paint_map& pixels) override
    {
        for (auto& cp : m_mainfield)
            cp.clean(pixels);
    }

    void set_text(const std::string& data)
    {
        m_w = data.length();
        m_mainfield.clear();

        for (size_t i = 0; i < m_w; i++)
            m_mainfield.emplace_back(point(m_x + i, m_y, data[i]));
    }
};

class text_box : public IGui_primitive
{
    rectangle m_box;
    text m_text;

public:
    text_box(size_t x = 0, size_t y = 0, const std::string& data = "")
    {
        m_x = x;
        m_y = y;
        m_w = data.length() + 2;
        m_h = 3;

        m_box = rectangle(m_x, m_y, m_w, m_h);
        m_text = text(m_x + 1, m_y + 1, data);
    }

    void move(size_t x, size_t y) override
    {
        m_x += x;
        m_y += y;

        m_box.move(x, y);
        m_text.move(x, y);
    }

    void get_data(paint_map& pixels) override
    {
        m_box.get_data(pixels);
        m_text.get_data(pixels);
    }

    void clean(paint_map& pixels) override
    {
        m_box.clean(pixels);
        m_text.clean(pixels);
    }

    void set_text(const std::string& data)
    {
        m_text.set_text(data);
        m_w = data.length() + 2;
        m_box = rectangle(m_x, m_y, m_w, m_h);
    }
};

class model
{
    using primitive_ptr = std::shared_ptr<IGui_primitive>;
    using paint_map = std::map<std::pair<size_t, size_t>, char>;
    using view_ptr = std::shared_ptr<view>;

    std::map<std::string, primitive_ptr> gui_primitives;
    std::mutex m_primitives_lock;

    paint_map m_screenshot;
    view_ptr m_view;

public:

    model(view_ptr view) : m_view(view) { }

    template<typename T, typename... Args>
    bool create_primitive(const std::string& name, Args... arguments)
    {
        auto new_primitive = std::make_shared<T>(T(std::forward<Args>(arguments)...));

        paint_map old_pixels;
        paint_map diff_pixels;

        new_primitive->get_data(diff_pixels);

        std::lock_guard<std::mutex> lock(m_primitives_lock);

        if (gui_primitives.contains(name))
        {
            auto old_primitive = gui_primitives[name];
            old_primitive->get_data(old_pixels);
            new_primitive->get_data(diff_pixels);

            //Make diff
            for (auto& p : old_pixels)
            {
                if (diff_pixels.contains(p.first))
                {
                    if (diff_pixels[p.first] == p.second)
                        diff_pixels.erase(p.first);
                }
                else
                {
                    diff_pixels[p.first] = IGui_primitive::empty_char;
                }
            }
        }

        gui_primitives[name] = new_primitive;
        m_view->paint(diff_pixels);

        return true;
    }

    auto get_primitive(const std::string& name)
    {
        std::unique_lock<std::mutex> lock(m_primitives_lock);
        return gui_primitives[name];
    }

    void move_primitive(const std::string& name, size_t x, size_t y)
    {
        std::lock_guard<std::mutex> lock(m_primitives_lock);

        if (gui_primitives.contains(name))
        {
            auto primitive = gui_primitives[name];

            paint_map old_pixels;
            paint_map diff_pixels;

            primitive->get_data(old_pixels);
            primitive->move(x, y);
            primitive->get_data(diff_pixels);

            //Make diff
            for (auto& p : old_pixels)
            {
                if (diff_pixels.contains(p.first))
                {
                    if (diff_pixels[p.first] == p.second)
                        diff_pixels.erase(p.first);
                }
                else
                {
                    diff_pixels[p.first] = IGui_primitive::empty_char;
                }
            }

            m_view->paint(diff_pixels);
        }
        else
        {
            //handle err
        }
    }
    
    void clean_primitive(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(m_primitives_lock);

        if (gui_primitives.contains(name))
        {
            paint_map pixels;

            gui_primitives[name]->clean(pixels);
            m_view->paint(pixels);
        }
        else
        {
            //handle err
        }
    }

    void draw_primitive(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(m_primitives_lock);

        if (gui_primitives.contains(name))
        {
            paint_map pixels;

            gui_primitives[name]->get_data(pixels);
            m_view->paint(pixels);
        }
        else
        {
            //handle err
        }
    }

    void clean_all()
    {
        std::lock_guard<std::mutex> lock(m_primitives_lock);

        for (auto& p : gui_primitives)
        {
            paint_map pixels;
            p.second->clean(pixels);
            m_view->paint(pixels);
        }
    }
};