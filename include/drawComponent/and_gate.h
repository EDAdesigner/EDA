// and_gate.h
#ifndef AND_GATE_H
#define AND_GATE_H

#include "component_drawer.h"
#include <wx/dc.h>

class AndGate : public IComponentDrawer {
public:
    void Draw(wxDC& dc, const wxPoint& pos) const override;
};

#endif // AND_GATE_H
