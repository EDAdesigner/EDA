// and_gate.cpp
#include "and_gate.h"

void AndGate::Draw(wxDC& dc, const wxPoint& pos) const {
    int gridX = (pos.x / 20) * 20;
    int gridY = (pos.y / 20) * 20;
    wxPoint snapPoint(gridX, gridY);
    dc.SetPen(wxPen(*wxBLACK, 4));

    // 绘制与门
    dc.DrawLine(snapPoint.x - 20, snapPoint.y - 20, snapPoint.x, snapPoint.y - 20);
    dc.DrawLine(snapPoint.x - 20, snapPoint.y - 20, snapPoint.x - 20, snapPoint.y + 20);
    dc.DrawLine(snapPoint.x - 20, snapPoint.y + 20, snapPoint.x, snapPoint.y + 20);

    wxPoint points[5] = {
        wxPoint(snapPoint.x, snapPoint.y - 20),
        wxPoint(snapPoint.x + 10, snapPoint.y - 17),
        wxPoint(snapPoint.x + 20, snapPoint.y),
        wxPoint(snapPoint.x + 10, snapPoint.y + 17),
        wxPoint(snapPoint.x, snapPoint.y + 20),
    };
    dc.DrawSpline(5, points);

    // 绘制输入输出口
    dc.DrawLine(snapPoint.x - 20, snapPoint.y + 10, snapPoint.x - 27, snapPoint.y + 10);
    dc.DrawLine(snapPoint.x - 20, snapPoint.y - 10, snapPoint.x - 27, snapPoint.y - 10);
    dc.DrawLine(snapPoint.x + 20, snapPoint.y, snapPoint.x + 25, snapPoint.y);
}
