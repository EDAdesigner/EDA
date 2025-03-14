// component_drawer.h
#ifndef COMPONENT_DRAWER_H
#define COMPONENT_DRAWER_H

#include <wx/dc.h>

class IComponentDrawer {
public:
    virtual ~IComponentDrawer() = default;
    virtual void Draw(wxDC& dc, const wxPoint& pos) const = 0;
};

#endif // COMPONENT_DRAWER_H
