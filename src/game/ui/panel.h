#ifndef PANEL_H
#define PANEL_H

#include "ui_base.h"

namespace shooter
{

class Panel : public UIBase
{
public:
    Panel() = default;

    void on_update(float dt) override { UIBase::on_update(dt); }
    void on_event() override { UIBase::on_event(); }
    void on_render() override { UIBase::on_render(); }
};

}    

#endif // !PANEL_H
