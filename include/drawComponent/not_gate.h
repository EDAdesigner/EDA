// not_gate.h
#ifndef NOT_GATE_H
#define NOT_GATE_H

#include "component_drawer.h"
#include <wx/dc.h>

class NotGate : public IComponentDrawer {
public:
    void Draw(wxDC& dc, const wxPoint& pos) const override;
};

#endif // NOT_GATE_H
