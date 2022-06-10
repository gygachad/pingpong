#pragma once

#include "model.h"
#include "view.h"

class controller
{
    model m_model;

    std::shared_ptr<view> m_view;

    using primitive_ptr = std::shared_ptr<IGui_primitive>;

public:
    controller(std::shared_ptr<view> client_view) : m_view(client_view)
    {
    }

    /*
    void clean(primitive_ptr primitive)
    {
        primitive->clean();
    }
    */
    void move(primitive_ptr primitive, size_t x, size_t y)
    {
        view::paint_map pixels;

        primitive->clean(pixels);
        primitive->move(x, y);
        primitive->get_draw_data(pixels);
        m_view->paint(pixels);
    }
    
    void draw(primitive_ptr primitive)
    {
        view::paint_map pixels;

        primitive->get_draw_data(pixels);
        m_view->paint(pixels);
    }
};