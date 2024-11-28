// not_gate.cpp
#include "not_gate.h"

void NotGate::Draw(wxDC& dc, const wxPoint& pos) const {
  dc.SetPen(wxPen(*wxBLACK, 4)); // 边框颜色和宽度;
                //绘制非门左侧部分
                dc.DrawLine(snapPoint.x - 20, snapPoint.y - 20, snapPoint.x + 12, snapPoint.y);
                dc.DrawLine(snapPoint.x - 20, snapPoint.y + 20, snapPoint.x - 20, snapPoint.y - 20);
                dc.DrawLine(snapPoint.x - 20, snapPoint.y + 20, snapPoint.x + 12, snapPoint.y);
                //绘制非门右侧部分
                dc.DrawCircle(snapPoint.x + 16, snapPoint.y, 4);
                //绘制输入输出口
                dc.DrawLine(snapPoint.x - 20, snapPoint.y, snapPoint.x - 27, snapPoint.y);
                dc.DrawLine(snapPoint.x + 20, snapPoint.y, snapPoint.x + 25, snapPoint.y);

}
