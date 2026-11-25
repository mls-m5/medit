#pragma once

#include "views/view.h"

class IScreen;

// A window region for tools, independent of the text-editor split list.
class SideBar : public View {
public:
    explicit SideBar(IView *parent)
        : View{parent} {}

    void draw(IScreen &screen) override;
};
