// or_gate.h
#ifndef OR_GATE_H
#define OR_GATE_H

#include "component_drawer.h"
#include <wx/dc.h>

class OrGate : public IComponentDrawer {
public:
    void Draw(wxDC& dc, const wxPoint& pos) const override;
};

#endif // OR_GATE_H
