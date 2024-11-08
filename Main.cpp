#include <wx/wx.h>
#include <wx/toolbar.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/artprov.h>
#include <vector>
#include <wx/dcbuffer.h>
#include <wx/frame.h>
#include <wx/treectrl.h>

class DrawPanel : public wxPanel {
    friend class MyFrame; // 假设 MyFrame 类存在
public:
    enum class Tool {
        NONE, AND_GATE, OR_GATE, NOT_GATE, NAND_GATE, NOR_GATE, XOR_GATE, XNOR_GATE,
    };
    wxBitmap* bitmap = nullptr;
    wxTimer* moveTimer;

    DrawPanel(wxWindow* parent)
        : wxPanel(parent), currentTool(Tool::NONE), dragging(false), moveTimer(new wxTimer(this)), connecting(false), startPoint(wxPoint(-1, -1)), connectionStartIndex(-1) {
        SetBackgroundColour(*wxWHITE);
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        bitmap = new wxBitmap(GetSize());

        Bind(wxEVT_PAINT, &DrawPanel::OnPaint, this);
        Bind(wxEVT_LEFT_DOWN, &DrawPanel::OnLeftDown, this);
        Bind(wxEVT_LEFT_UP, &DrawPanel::OnLeftUp, this);
        Bind(wxEVT_MOTION, &DrawPanel::OnMouseMove, this);
        Bind(wxEVT_RIGHT_DOWN, &DrawPanel::OnRightDown, this);
        Bind(wxEVT_SIZE, &DrawPanel::OnSize, this);
        Bind(wxEVT_TIMER, &DrawPanel::OnMoveTimer, this);
    }

    ~DrawPanel() {
        delete bitmap;
        delete moveTimer;
    }

    void SetCurrentTool(Tool tool) {
        currentTool = tool;
    }

    Tool currentTool;
    std::vector<std::pair<Tool, wxPoint>> components;
    std::vector<int> selectedComponents;
    std::vector<std::pair<Tool, wxPoint>> copiedComponents;
    std::vector<std::pair<int, int>> connections; // 存储连接线的组件索引
    bool dragging;
    bool connecting;
    int draggedComponentIndex;
    wxPoint dragStartPos;
    wxPoint componentOffset; // 新增：记录元件相对于鼠标的偏移
    wxPoint startPoint; // 连线起始点
    int connectionStartIndex; // 声明变量

    void OnPaint(wxPaintEvent& event) {
        wxBufferedPaintDC dc(this);
        PrepareDC(dc);
        dc.SetBackground(*wxWHITE_BRUSH);
        dc.Clear();
        Render(dc);
    }

    void Render(wxDC& dc) {
        DrawGrid(dc);
        for (const auto& component : components) {
            DrawComponent(dc, component.first, component.second);
        }
        DrawConnections(dc);
    }

    void DrawGrid(wxDC& dc) {
        dc.SetPen(wxPen(wxColour(200, 200, 200), 1, wxPENSTYLE_DOT));
        for (int i = 0; i < GetSize().GetWidth(); i += 20) {
            dc.DrawLine(i, 0, i, GetSize().GetHeight());
        }
        for (int j = 0; j < GetSize().GetHeight(); j += 20) {
            dc.DrawLine(0, j, GetSize().GetWidth(), j);
        }
    }

    void DrawComponent(wxDC& dc, Tool tool, const wxPoint& pos) {
        int gridX = (pos.x / 20) * 20;
        int gridY = (pos.y / 20) * 20;
        wxPoint snapPoint(gridX, gridY);

        // 示例：绘制与门
        if (tool == Tool::AND_GATE) {
            dc.SetPen(wxPen(*wxBLACK, 4));
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
            dc.DrawLine(snapPoint.x - 20, snapPoint.y + 10, snapPoint.x - 27, snapPoint.y + 10);
            dc.DrawLine(snapPoint.x - 20, snapPoint.y - 10, snapPoint.x - 27, snapPoint.y - 10);
            dc.DrawLine(snapPoint.x + 20, snapPoint.y, snapPoint.x + 25, snapPoint.y);
        }
    }

    void DrawConnections(wxDC& dc) {
        dc.SetPen(wxPen(*wxBLUE, 2));
        for (const auto& connection : connections) {
            wxPoint start = components[connection.first].second;
            wxPoint end = components[connection.second].second;
            DrawGridAlignedLine(dc, start, end);
        }
    }

    void DrawGridAlignedLine(wxDC& dc, const wxPoint& start, const wxPoint& end) {
        wxPoint gridStart((start.x / 20) * 20, (start.y / 20) * 20);
        wxPoint gridEnd((end.x / 20) * 20, (end.y / 20) * 20);

        if (gridStart.x == gridEnd.x || gridStart.y == gridEnd.y) {
            dc.DrawLine(gridStart, gridEnd);
        }
        else {
            wxPoint midPoint(gridEnd.x, gridStart.y);
            dc.DrawLine(gridStart, midPoint);
            dc.DrawLine(midPoint, gridEnd);
        }
    }

    void OnLeftDown(wxMouseEvent& event) {
        wxPoint pos = event.GetPosition();

        for (size_t i = 0; i < components.size(); ++i) {
            if (abs(components[i].second.x - pos.x) < 20 && abs(components[i].second.y - pos.y) < 20) {
                if (connecting) {
                    connections.emplace_back(connectionStartIndex, i);
                    connecting = false;
                    Refresh();
                    return;
                }
                dragging = true;
                draggedComponentIndex = i;
                dragStartPos = pos;
                componentOffset = pos - components[i].second; // 记录偏移
                CaptureMouse();
                return;
            }
        }

        if (!dragging && currentTool != Tool::NONE) {
            components.emplace_back(currentTool, pos);
            Refresh();
        }
    }

    void OnLeftUp(wxMouseEvent& event) {
        if (dragging) {
            dragging = false;
            ReleaseMouse();
        }
    }

    void OnMouseMove(wxMouseEvent& event) {
        if (dragging) {
            wxPoint pos = event.GetPosition();
            wxPoint newComponentPos = pos - componentOffset; // 使用偏移计算新位置
            wxPoint& componentPos = components[draggedComponentIndex].second;
            componentPos = newComponentPos;

            // 更新连接线位置
            UpdateConnections(draggedComponentIndex, newComponentPos - componentPos);

            Refresh();
            Update();
        }
    }

    void UpdateConnections(int index, const wxPoint& offset) {
        for (auto& connection : connections) {
            if (connection.first == index || connection.second == index) {
                if (connection.first == index) {
                    components[connection.first].second += offset;
                }
                if (connection.second == index) {
                    components[connection.second].second += offset;
                }
            }
        }
    }

    void OnRightDown(wxMouseEvent& event) {
        wxPoint pos = event.GetPosition();
        bool componentFound = false;
        int componentToDelete = -1;

        for (size_t i = 0; i < components.size(); ++i) {
            if (abs(components[i].second.x - pos.x) < 20 && abs(components[i].second.y - pos.y) < 20) {
                componentFound = true;
                componentToDelete = i;
                break;
            }
        }

        if (componentFound) {
            wxMenu menu;
            menu.Append(wxID_ANY, "Delete");
            menu.Append(wxID_ANY, "Connect");

            Bind(wxEVT_MENU, [this, componentToDelete](wxCommandEvent&) {
                if (componentToDelete != -1) {
                    components.erase(components.begin() + componentToDelete);
                    Refresh();
                }
                }, wxID_ANY);

            Bind(wxEVT_MENU, [this, componentToDelete](wxCommandEvent&) {
                if (componentToDelete != -1) {
                    connecting = true;
                    connectionStartIndex = componentToDelete;
                }
                }, wxID_ANY);

            PopupMenu(&menu);
        }
    }

    void OnSize(wxSizeEvent& event) {
        Refresh();
        event.Skip();
    }

    void OnMoveTimer(wxTimerEvent&) {
        Refresh();
        moveTimer->Stop();
    }

    void SelectAll() {
        selectedComponents.clear();
        for (size_t i = 0; i < components.size(); ++i) {
            selectedComponents.push_back(i);
        }
        Refresh();
    }

    void CutSelected() {
        for (auto it = selectedComponents.rbegin(); it != selectedComponents.rend(); ++it) {
            components.erase(components.begin() + *it);
        }
        selectedComponents.clear();
        Refresh();
    }

    void CopySelected() {
        copiedComponents.clear();
        for (int index : selectedComponents) {
            if (index >= 0 && index < components.size()) {
                copiedComponents.push_back(components[index]);
            }
        }
    }

    void PasteCopied() {
        for (const auto& component : copiedComponents) {
            wxPoint newPosition = component.second + wxPoint(10, 10);
            components.push_back({ component.first, newPosition });
        }
        Refresh();
    }
};