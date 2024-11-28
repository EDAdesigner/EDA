#include<wx/wx.h>
#include <wx/toolbar.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/artprov.h>
#include <vector>
#include <wx/dcbuffer.h>
#include <wx/frame.h>       // 包含框架窗口相关功能
#include <wx/treectrl.h>    // 包含树控件的相关功能
#include <fstream>
#include <nlohmann/json.hpp>
#include <wx/dc.h>
//#include <wx/pen.h>

using json = nlohmann::json;

class DrawPanel : public wxPanel {
    friend class MyFrame; // 声明 MyFrame 为友元类,使得wxframe能够访问DrawPanel的私有方法
public:
    enum class Tool { NONE, AND_GATE, OR_GATE, NOT_GATE, NAND_GATE, NOR_GATE, XOR_GATE, XNOR_GATE, }; // 定义工具类型，包括无工具、与门、或门和非门 // 定义工具类型，包括无工具、与门、或门和非门
    wxBitmap* bitmap = nullptr;// 新增位图指针
    wxTimer* moveTimer;
    int connectionStartIndex;
    double scaleFactor = 0.5;//缩小比例，在每个映射时除以该因子

    DrawPanel(wxWindow* parent)
        : wxPanel(parent), currentTool(Tool::NONE), dragging(false), moveTimer(new wxTimer(this)), connecting(false), startPoint(wxPoint(-1, -1)), connectionStartIndex(-1) {
        // 构造函数，初始化面板及背景颜色
        SetBackgroundColour(*wxWHITE);
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        bitmap = new wxBitmap(GetSize()); // 初始化位图
        // 绑定事件
        Bind(wxEVT_PAINT, &DrawPanel::OnPaint, this); // 绘制事件
        Bind(wxEVT_LEFT_DOWN, &DrawPanel::OnLeftDown, this); // 左键按下事件
        Bind(wxEVT_LEFT_UP, &DrawPanel::OnLeftUp, this); // 左键抬起事件
        Bind(wxEVT_MOTION, &DrawPanel::OnMouseMove, this); // 鼠标移动事件
        Bind(wxEVT_RIGHT_DOWN, &DrawPanel::OnRightDown, this); // 右键按下事件
        Bind(wxEVT_SIZE, &DrawPanel::OnSize, this); // 面板大小变化事件
        Bind(wxEVT_TIMER, &DrawPanel::OnMoveTimer, this);// 绑定定时器事件

    }

    ~DrawPanel() {
        delete bitmap; // 释放位图内存
        delete moveTimer; // 释放定时器内存

    }

    void SetCurrentTool(Tool tool) {
        currentTool = tool; // 设置当前选择的工具
    }

    //private:
    Tool currentTool; // 当前工具
    std::vector<std::pair<Tool, wxPoint>> components; // 存储已添加的组件及其位置
    std::vector<int> selectedComponents;// 存储选中的组件
    std::vector<std::pair<Tool, wxPoint>> copiedComponents; // 存储复制的组件
    std::vector<std::pair<int, int>> connections; // 存储连接线的组件索引
    bool dragging; // 标记是否正在拖动组件
    bool connecting;
    int draggedComponentIndex; // 被拖动的组件索引
    wxPoint dragStartPos;
    wxPoint componentOffset; // 新增：记录元件相对于鼠标的偏移
    wxPoint startPoint; // 连线起始点
    wxPoint m_mousePos;// 存储鼠标的网格位置



    void OnPaint(wxPaintEvent& event) {
        wxBufferedPaintDC dc(this);
        PrepareDC(dc);
        dc.SetBackground(*wxWHITE_BRUSH);
        dc.Clear();
        dc.SetUserScale(scaleFactor, scaleFactor);
        Render(dc);
        if (m_mousePos.x >= 0 && m_mousePos.y >= 0) {
            // 绘制蓝色高亮圆圈
            dc.SetPen(wxPen(wxColour(0, 0, 255), 1));  // 设置蓝色边框
            dc.SetBrush(wxBrush(wxColour(0, 0, 255), wxBRUSHSTYLE_TRANSPARENT));  // 设置透明填充
            dc.DrawCircle(m_mousePos, 3);  // 绘制半径为10的圆圈
        }
    }

    void Render(wxDC& dc) {
        //DrawGrid(dc);  // 绘制网格
        for (size_t i = 0; i < components.size(); ++i) {
            const auto& component = components[i];
            DrawComponent(dc, component.first, component.second, i);
        }
        DrawConnections(dc);  // 绘制连接线
    }


    /*void DrawGrid(wxDC& dc) {
        dc.SetPen(wxPen(wxColour(200, 200, 200), 1, wxPENSTYLE_DOT));
        for (int i = 0; i < GetSize().GetWidth(); i += 20) {
            dc.DrawLine(i, 0, i, GetSize().GetHeight());
        }
        for (int j = 0; j < GetSize().GetHeight(); j += 20) {
            dc.DrawLine(0, j, GetSize().GetWidth(), j);
        }
    }*/
    
    void DrawComponent(wxDC& dc, Tool tool, const wxPoint& pos, size_t index) {
        // 将位置映射到网格
        int gridX = (pos.x / 20) * 20/scaleFactor;
        int gridY = (pos.y / 20) * 20/scaleFactor;
        wxPoint snapPoint(gridX, gridY);

        // 根据工具类型选择对应的 JSON 文件
        std::string toolName;
        switch (tool) {
        case Tool::AND_GATE:
            toolName = "AND_GATE.json";
            break;
        case Tool::OR_GATE:
            toolName = "OR_GATE.json";
            break;
        case Tool::NOT_GATE:
            toolName = "NOT_GATE.json";
            break;
        case Tool::NAND_GATE:
            toolName = "NAND_GATE.json";
            break;
        case Tool::NOR_GATE:
            toolName = "NOR_GATE.json";
            break;
        case Tool::XOR_GATE:
            toolName = "XOR_GATE.json";
            break;
        case Tool::XNOR_GATE:
            toolName = "XNOR_GATE.json";
            break;
        default:
            return;
        }

        // 加载对应的 JSON 文件
        std::ifstream file("tools/" + toolName);
        if (!file.is_open()) {
            wxLogError("Cannot open the JSON file for %s", toolName);
            return;
        }

        json componentJson;
        file >> componentJson;
        file.close();

        // 记录是否有输入或输出部分被高亮
        bool highlightInput = false;
        bool highlightOutput = false;

        // 绘制直线部分
        dc.SetPen(wxPen(*wxBLACK, 2)); // 设置边框颜色和宽度
        int i = 0;
        for (const auto& line : componentJson["lines"]) {
            i++;
            wxPoint start(line["start"][0].get<int>(), line["start"][1].get<int>());
            wxPoint end(line["end"][0].get<int>(), line["end"][1].get<int>());
            dc.DrawLine(snapPoint.x + start.x, snapPoint.y + start.y, snapPoint.x + end.x, snapPoint.y + end.y);
        }
        // 绘制曲线部分（如有）
        if (componentJson.contains("splines")) {
            for (const auto& spline : componentJson["splines"]) {
                std::vector<wxPoint> points;
                for (const auto& point : spline["points"]) {
                    points.push_back(wxPoint(snapPoint.x + point[0].get<int>(), snapPoint.y + point[1].get<int>()));
                }
                dc.DrawSpline(points.size(), points.data());
            }
        }

        // 绘制圆形部分（如有）
        if (componentJson.contains("circle")) {
            auto circle = componentJson["circle"];
            int cx = circle["center"][0].get<int>();
            int cy = circle["center"][1].get<int>();
            int radius = circle["radius"].get<int>();
            dc.DrawCircle(snapPoint.x + cx, snapPoint.y + cy, radius);
        }

        // 绘制输入口
        if (componentJson.contains("inputs")) {
            for (const auto& input : componentJson["inputs"]) {
                wxPoint start(input["start"][0].get<int>(), input["start"][1].get<int>());
                wxPoint end(input["end"][0].get<int>(), input["end"][1].get<int>());
                // 检查鼠标是否在输入端
                bool isMouseOver = IsMouseOverLine(start + snapPoint, end + snapPoint, m_mousePos);

                // 设置绘制样式：如果鼠标在输入端，使用红色高亮；否则使用默认黑色
                if (isMouseOver) {
                    dc.SetPen(wxPen(wxColour(255, 0, 0), 7));
                    highlightInput = true;
                }
                else {
                    dc.SetPen(wxPen(*wxBLACK, 2));
                }

                // 绘制输入端线段
                dc.DrawLine(snapPoint.x + start.x, snapPoint.y + start.y, snapPoint.x + end.x, snapPoint.y + end.y);
            }
            // 恢复默认线条样式
            dc.SetPen(wxPen(*wxBLACK, 2));
        }


        // 绘制输出口
        if (componentJson.contains("outputs")) {
            for (const auto& output : componentJson["outputs"]) {
                wxPoint start(output["start"][0].get<int>(), output["start"][1].get<int>());
                wxPoint end(output["end"][0].get<int>(), output["end"][1].get<int>());

                // 检查鼠标是否在输出端
                if (IsMouseOverLine(start + snapPoint, end + snapPoint, m_mousePos)) {
                    dc.SetPen(wxPen(wxColour(255, 0, 0), 7)); // 红色，粗细7
                    highlightOutput = true;
                }
                else {
                    dc.SetPen(wxPen(*wxBLACK, 2)); // 默认黑色，粗细2
                }

                // 绘制输出端线段
                dc.DrawLine(snapPoint.x + start.x, snapPoint.y + start.y, snapPoint.x + end.x, snapPoint.y + end.y);
            }
        }
        // 恢复默认线条样式
        dc.SetPen(wxPen(*wxBLACK, 2));
    }

    bool IsMouseOverLine(const wxPoint& start, const wxPoint& end, const wxPoint& mousePos) {
        // 计算线段的矩形区域
        int minX = std::min(start.x, end.x);
        int maxX = std::max(start.x, end.x);
        int minY = std::min(start.y, end.y);
        int maxY = std::max(start.y, end.y);

        // 判断鼠标是否在该区域内，允许一定的误差

        return (mousePos.x >= minX - 5 && mousePos.x <= maxX + 5 &&
            mousePos.y >= minY - 5 && mousePos.y <= maxY + 5);
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

        // 尝试找到鼠标按下时所在的组件
        for (size_t i = 0; i < components.size(); ++i) {
            // 简单的碰撞检测，如果鼠标位置接近组件位置，就认为找到了该组件
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
                componentOffset = pos - components[i].second;  // 记录鼠标按下时相对组件的位置
                CaptureMouse();  // 捕获鼠标
                return;
            }
        }

        // 如果没有找到组件并且有选中的工具，创建一个新组件
        if (!dragging && currentTool != Tool::NONE) {
            components.emplace_back(currentTool, pos);
            Refresh();
        }
    }

    void OnLeftUp(wxMouseEvent& event) {
        // 释放拖动标记
        if (dragging) {
            dragging = false;
            ReleaseMouse(); // 释放鼠标捕获
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
        wxPoint mousePos = event.GetPosition();
        // 根据缩放因子计算鼠标在缩放后的坐标
        mousePos.x /= scaleFactor;
        mousePos.y /= scaleFactor;
        m_mousePos = wxPoint(mousePos.x / 20 * 20, mousePos.y / 20 * 20);
        // 刷新画面来重新绘制高亮圆圈
        Refresh();

    }


    std::string GetComponentFileName(Tool tool) {
        switch (tool) {
        case Tool::AND_GATE: return "AND_GATE.json";
        case Tool::OR_GATE: return "OR_GATE.json";
        case Tool::NOT_GATE: return "NOT_GATE.json";
        case Tool::NAND_GATE: return "NAND_GATE.json";
        case Tool::NOR_GATE: return "NOR_GATE.json";
        case Tool::XOR_GATE: return "XOR_GATE.json";
        case Tool::XNOR_GATE: return "XNOR_GATE.json";
        default: return "";
        }
    }

    wxPoint GetSnapPoint(const wxPoint& pos) {
        // 将位置映射到网格上 (网格大小为 20)
        int gridX = (pos.x / 20) * 20;
        int gridY = (pos.y / 20) * 20;
        return wxPoint(gridX, gridY);  // 返回对齐后的点
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


    void OnMoveTimer(wxTimerEvent&) {
        Refresh(); // 刷新绘图
        moveTimer->Stop(); // 停止定时器
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
        Refresh(); // 面板大小改变时刷新绘图
        event.Skip(); // 继续处理其他事件
    }

    // 选择所有组件
    void SelectAll() {
        selectedComponents.clear(); // 清空之前的选择
        for (size_t i = 0; i < components.size(); ++i) {
            selectedComponents.push_back(i); // 将所有组件索引添加到选中列表
        }
        Refresh(); // 刷新绘图
    }

    // 删除选中的组件
    void CutSelected() {
        for (auto it = selectedComponents.rbegin(); it != selectedComponents.rend(); ++it) {
            components.erase(components.begin() + *it); // 从 components 中删除
        }
        selectedComponents.clear(); // 清空选中的组件
        Refresh(); // 刷新绘图
    }

    //复制选中的组件
    void CopySelected() {
        copiedComponents.clear(); // 清空之前的复制内容
        for (int index : selectedComponents) {
            if (index >= 0 && index < components.size()) {
                copiedComponents.push_back(components[index]); // 复制组件
            }
        }
    }

    //粘贴选中的组件
    void PasteCopied() {
        for (const auto& component : copiedComponents) {
            // 在复制组件的位置上进行粘贴，稍微调整位置
            wxPoint newPosition = component.second + wxPoint(10, 10); // 偏移位置
            components.push_back({ component.first, newPosition }); // 添加到组件列表中
        }
        Refresh(); // 刷新绘图
    }

};