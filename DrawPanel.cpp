#include <wx/wx.h>
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
#include"Component.cpp"
//#include <wx/pen.h>

using json = nlohmann::json;

class DrawPanel : public wxPanel {
    friend class MyFrame; // 声明 MyFrame 为友元类,使得wxframe能够访问DrawPanel的私有方法
public:
    wxBitmap* bitmap = nullptr;// 新增位图指针
    wxTimer* moveTimer;
    wxPoint connectionStartPosition;
    double scaleFactor = 0.5;//缩小比例，在每个映射时除以该因子
    Component::Tool currentTool; // 当前工具
    std::vector<Component> components;  // 存储所有组件对象
    std::vector<std::pair<wxPoint, wxPoint>> connections; // 存储连接线的组件索引
    bool dragging; // 标记是否正在拖动组件
    bool connecting;
    int draggedComponentIndex; // 被拖动的组件索引

    wxPoint dragStartPos;//存储拖动开始位置（每次拖动后会更新），用于更新连线

    wxPoint componentOffset; // 新增：记录元件相对于鼠标的偏移
    //wxPoint startPoint; // 连线起始点
    wxPoint m_mousePos;// 存储鼠标的网格位置


    DrawPanel(wxWindow* parent)
        : wxPanel(parent), currentTool(Component::Tool::NONE), dragging(false), moveTimer(new wxTimer(this)), connecting(false), connectionStartPosition(wxPoint(-1,-1)) {
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

    void SetCurrentTool(Component::Tool tool) {
        currentTool = tool; // 设置当前选择的工具
    }


    void OnPaint(wxPaintEvent& event) {
        wxBufferedPaintDC dc(this);
        PrepareDC(dc);
        dc.SetBackground(*wxWHITE_BRUSH);
        dc.Clear();
        dc.SetUserScale(scaleFactor, scaleFactor);
     
        // 绘制网格点
        DrawGrid(dc);

        Render(dc);

        if (m_mousePos.x >= 0 && m_mousePos.y >= 0) {
            // 绘制蓝色高亮圆圈
            dc.SetPen(wxPen(wxColour(0, 0, 255), 1));  // 设置蓝色边框
            dc.SetBrush(wxBrush(wxColour(0, 0, 255), wxBRUSHSTYLE_TRANSPARENT));  // 设置透明填充
            dc.DrawCircle(m_mousePos, 3);  // 绘制半径为10的圆圈
        }
    }


    void Render(wxDC& dc) {
        for (size_t i = 0; i < components.size(); ++i) {
            auto& component = components[i];
            component.Draw(dc, m_mousePos);  // 使用 Component 类的 Draw 方法来绘制
        }
            DrawConnections(dc);  // 绘制连接线
    }

    //绘制网格点
    void DrawGrid(wxDC& dc) {
        const int gridSize = 20;  // 网格大小，20 像素
        const int dotRadius = 2;  // 网格点的半径，设置为 2 像素

        dc.SetPen(wxPen(wxColour(220, 220, 220), 1));  // 设置浅灰色的网格点边框
        dc.SetBrush(wxBrush(wxColour(220, 220, 220)));  // 设置网格点的填充颜色，浅灰色

        // 绘制水平和垂直方向的网格点
        for (int y = 0; y < GetSize().y / scaleFactor; y += gridSize) {
            for (int x = 0; x < GetSize().x / scaleFactor; x += gridSize) {
                // 在每个网格单元的中心绘制一个小圆点
                dc.DrawCircle(x, y, dotRadius);
            }
        }
    }


    void DrawConnections(wxDC& dc) {
        dc.SetPen(wxPen(*wxBLACK, 3));  // 设置画笔颜色和粗细
        for (const auto& connection : connections) {
            // 获取起始组件的第一个引脚
            wxPoint startPin = connection.first; // 获取第一个输出引脚
            // 获取目标组件的第一个引脚
            wxPoint endPin = connection.second;  // 获取第一个输入引脚
            DrawGridAlignedLine(dc,startPin, endPin);  // 绘制连接线
        }
    }



    void DrawGridAlignedLine(wxDC& dc, const wxPoint& start, const wxPoint& end) {
        // 计算对齐到网格的起点和终点
        wxPoint gridStart((start.x / 20) * 20, (start.y / 20) * 20);
        wxPoint gridEnd((end.x / 20) * 20, (end.y / 20) * 20);

        // 如果 x 坐标相同，说明连线垂直
        if (gridStart.x == gridEnd.x) {
            dc.DrawLine(gridStart, gridEnd);
        }
        // 如果 y 坐标相同，说明连线水平
        else if (gridStart.y == gridEnd.y) {
            dc.DrawLine(gridStart, gridEnd);
        }
        else {
            // 否则绘制曼哈顿连线，先水平再垂直
            wxPoint midPoint(gridEnd.x, gridStart.y);  // 水平到垂直的连接点
            dc.DrawLine(gridStart, midPoint);
            dc.DrawLine(midPoint, gridEnd);
        }
    }


    void OnLeftDown(wxMouseEvent& event) {
        wxPoint pos = GetSnapPoint(event.GetPosition());
        // 如果正在连接引脚
        if (connecting) {
            for (size_t i = 0; i < components.size(); ++i) {
                // 如果点击在组件的输入引脚或输出引脚上，记录连接起始或终止的组件
                if (components[i].IsMouseOverPin(pos)) {
                    if (connectionStartPosition == wxPoint(-1,-1)) {
                        connectionStartPosition = components[i].GetPinPosition(pos);
                    }
                    else {
                        wxPoint pinPosition = components[i].GetPinPosition(pos);
                        connections.push_back(std::pair<wxPoint, wxPoint>(connectionStartPosition, pinPosition));
                        connectionStartPosition = wxPoint(-1, -1);  // 重置连接起始组件
                        connecting = false;
                    }
                    Refresh();
                    return;
                }
            }
        }

        // 判断是否点击到已有组件并准备拖拽
        for (size_t i = 0; i < components.size(); ++i) {
            if (components[i].IsMouseOverComponent(pos)){
                // 记录拖拽的组件和起始位置
                dragging = true;
                draggedComponentIndex = i;
                dragStartPos = pos;
                componentOffset = pos - components[i].position;  // 计算鼠标与组件位置的偏移量，注：组件位置认为是中心位置，而鼠标位置不一定是组件位置（是范围内任意一点）
                CaptureMouse();
                return;
            }
        }

        // 如果点击的是空白区域，创建新的组件
        if (!dragging && !connecting) {
            // 假设此时选择了 AND_GATE 作为新组件
            wxPoint newPos = pos;  // 设置组件初始位置为点击位置
            Component newComponent(currentTool, newPos);

            // 从 JSON 文件加载组件的引脚信息
            std::string componentFileName = newComponent.GetComponentFileName(currentTool);
            newComponent.LoadPinsFromJson("tools/" + componentFileName);

            // 将新组件加入到组件列表中
            components.push_back(newComponent);

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
            wxPoint pos = GetSnapPoint(event.GetPosition());
            wxPoint newComponentPos = pos - componentOffset;//鼠标位置与元件位置（中心）相差一个Offset偏移量
            wxPoint prepos = components[draggedComponentIndex].position;
            // 更新所有与该组件连接的线条
            UpdateConnections(draggedComponentIndex, pos - dragStartPos, prepos);

            dragStartPos = pos;

            // 更新组件的新位置
            components[draggedComponentIndex].position = newComponentPos;


            // 刷新绘制
            Refresh();
        }

        m_mousePos = GetSnapPoint(event.GetPosition());
        Refresh();
    }


    wxPoint GetSnapPoint(const wxPoint& pos) {
        // 将位置映射到网格上 (网格大小为 20)
        wxPoint temp = pos;
        temp.x /= scaleFactor;
        temp.y /= scaleFactor;
        int gridX = (temp.x / 20) * 20;
        int gridY = (temp.y / 20) * 20;

        return wxPoint(gridX, gridY);  // 返回对齐后的点
    }

    void UpdateConnections(int draggedComponentIndex, const wxPoint& offset,wxPoint prepos) {
        // 遍历当前元件存在的所有的连接
        for (auto& connection : connections) {
            for (const auto& pin : components[draggedComponentIndex].pins) {
                wxPoint currentPinPosition = pin.first + prepos;
                if (connection.first == currentPinPosition) {
                    connection.first += offset;
                }
                if (connection.second == currentPinPosition) {
                    connection.second += offset;
                }
            }
        }

    }


    void OnMoveTimer(wxTimerEvent&) {
        Refresh(); // 刷新绘图
        moveTimer->Stop(); // 停止定时器
    }

    void OnRightDown(wxMouseEvent& event) {
        wxPoint pos = GetSnapPoint(event.GetPosition());
        bool componentFound = false;
        int componentSelected = -1;

        for (size_t i = 0; i < components.size(); ++i) {
            if (components[i].IsMouseOverComponent(pos)) {
                componentFound = true;
                componentSelected = i;
                break;
            }
        }

        if (componentFound) {
            wxMenu menu;
            
            const int deleteId = wxNewId();
            //const int connectId = wxNewId();
            menu.Append(deleteId, "Delete");
            //menu.Append(connectId, "Connect");

            // 删除操作
            Bind(wxEVT_MENU, [this, componentSelected](wxCommandEvent&) {
                if (componentSelected != -1) {
                    components.erase(components.begin() + componentSelected);
                    Refresh();
                }
                }, deleteId);

            PopupMenu(&menu);
        }
    }


    void OnSize(wxSizeEvent& event) {
        Refresh(); // 面板大小改变时刷新绘图
        event.Skip(); // 继续处理其他事件
    }

};