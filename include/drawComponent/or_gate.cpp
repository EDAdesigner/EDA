// or_gate.cpp
#include "or_gate.h"

void OrGate::Draw(wxDC& dc, const wxPoint& pos) const {
  dc.SetPen(wxPen(*wxBLACK, 4)); // 边框颜色和宽度;
                //绘制或门左侧部分
                wxPoint leftPoints[3] = {
                    wxPoint(snapPoint.x - 20, snapPoint.y - 20),
                    wxPoint(snapPoint.x - 10, snapPoint.y),
                    wxPoint(snapPoint.x - 20, snapPoint.y + 20),
                };
                dc.DrawSpline(3, leftPoints);
                //绘制或门右侧部分
                wxPoint rightPoints[5] = {
                    wxPoint(snapPoint.x - 20, snapPoint.y - 20),
                    wxPoint(snapPoint.x, snapPoint.y - 18),
                    wxPoint(snapPoint.x + 25, snapPoint.y),
                    wxPoint(snapPoint.x, snapPoint.y + 18),
                    wxPoint(snapPoint.x - 20, snapPoint.y + 20)
                };
                dc.DrawSpline(5, rightPoints);
                //绘制输入输出口
                dc.DrawLine(snapPoint.x - 14, snapPoint.y + 10, snapPoint.x - 25, snapPoint.y + 10);
                dc.DrawLine(snapPoint.x - 14, snapPoint.y - 10, snapPoint.x - 25, snapPoint.y - 10);
                dc.DrawLine(snapPoint.x + 20, snapPoint.y, snapPoint.x + 25, snapPoint.y);
}
