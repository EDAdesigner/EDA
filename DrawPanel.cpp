#include<wx/wx.h>
#include <wx/toolbar.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/artprov.h>
#include <vector>
#include <wx/frame.h>       // 包含框架窗口相关功能
#include <wx/treectrl.h>    // 包含树控件的相关功能


class DrawPanel : public wxPanel {
    friend class MyFrame; // 声明 MyFrame 为友元类,使得wxframe能够访问DrawPanel的私有方法
public:
    enum class Tool { NONE, AND_GATE, OR_GATE, NOT_GATE }; // 定义工具类型，包括无工具、与门、或门和非门
    wxBitmap* bitmap = nullptr;// 新增位图指针

    DrawPanel(wxWindow* parent)
        : wxPanel(parent), currentTool(Tool::NONE), dragging(false) {
        // 构造函数，初始化面板及背景颜色
        SetBackgroundColour(*wxWHITE);
        bitmap = new wxBitmap(GetSize()); // 初始化位图
        // 绑定事件
        Bind(wxEVT_PAINT, &DrawPanel::OnPaint, this); // 绘制事件
        Bind(wxEVT_LEFT_DOWN, &DrawPanel::OnLeftDown, this); // 左键按下事件
        Bind(wxEVT_LEFT_UP, &DrawPanel::OnLeftUp, this); // 左键抬起事件
        Bind(wxEVT_MOTION, &DrawPanel::OnMouseMove, this); // 鼠标移动事件
        Bind(wxEVT_RIGHT_DOWN, &DrawPanel::OnRightDown, this); // 右键按下事件
        Bind(wxEVT_SIZE, &DrawPanel::OnSize, this); // 面板大小变化事件
    }

    ~DrawPanel() {
        delete bitmap; // 释放位图内存
    }

    void SetCurrentTool(Tool tool) {
        currentTool = tool; // 设置当前选择的工具
    }

    //private:
    Tool currentTool; // 当前工具
    std::vector<std::pair<Tool, wxPoint>> components; // 存储已添加的组件及其位置
    std::vector<int> selectedComponents;// 存储选中的组件
    std::vector<std::pair<Tool, wxPoint>> copiedComponents; // 存储复制的组件
    bool dragging; // 标记是否正在拖动组件
    int draggedComponentIndex; // 被拖动的组件索引
    wxPoint dragStartPos; // 拖动开始位置

    void OnPaint(wxPaintEvent& event) {
        if (!bitmap || bitmap->GetSize() != GetSize()) {
            delete bitmap;  // 删除旧位图
            bitmap = new wxBitmap(GetSize()); // 创建新的位图
        }
        Render(*bitmap); // 每次绘制都更新位图
        wxPaintDC dc(this);
        dc.DrawBitmap(*bitmap, 0, 0); // 将位图绘制到面板上
    }

    void Render(wxBitmap& bitmap) {
        wxMemoryDC memDC(bitmap); // 使用内存DC绘制到位图
        memDC.SetBackground(*wxWHITE_BRUSH);
        memDC.Clear();
        DrawGrid(memDC);
        for (const auto& component : components) {
            DrawComponent(memDC, component.first, component.second);
        }
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
        // 根据工具类型绘制对应的组件
        if (tool == Tool::AND_GATE) {
            wxPoint points[4] = {
                wxPoint(snapPoint.x - 20, snapPoint.y - 20), // 左上
                wxPoint(snapPoint.x , snapPoint.y - 20), // 右上
                wxPoint(snapPoint.x , snapPoint.y + 20), // 中下
                wxPoint(snapPoint.x - 20, snapPoint.y + 20)  // 左下
            };
            dc.SetPen(wxPen(*wxBLACK, 4)); // 边框颜色和宽度
            dc.DrawPolygon(4, points); // 绘制与门左侧部分
            dc.DrawArc(snapPoint.x, snapPoint.y + 20, snapPoint.x, snapPoint.y - 20, snapPoint.x, snapPoint.y); // 绘制圆边
            wxRect rect(snapPoint.x - 10, snapPoint.y - 17, 12, 35);
            dc.SetPen(wxPen(*wxWHITE, 2)); // 边框颜色和宽度
            dc.SetBrush(wxBrush(*wxWHITE)); // 设置为白色刷子以覆盖
            dc.DrawRectangle(rect); // 绘制覆盖矩形
            dc.SetPen(wxPen(*wxBLACK, 4)); // 边框颜色和宽度
            dc.DrawLine(snapPoint.x - 20, snapPoint.y + 10, snapPoint.x - 27, snapPoint.y + 10);
            dc.DrawLine(snapPoint.x - 20, snapPoint.y - 10, snapPoint.x - 27, snapPoint.y - 10);
            dc.DrawLine(snapPoint.x + 20, snapPoint.y, snapPoint.x + 25, snapPoint.y);
        }
        else if (tool == Tool::OR_GATE) {
            dc.SetPen(wxPen(*wxBLACK, 4)); // 边框颜色和宽度;
            wxPoint rightPoints[5] = {
                wxPoint(snapPoint.x - 20, snapPoint.y - 20),
                wxPoint(snapPoint.x, snapPoint.y - 18),
                wxPoint(snapPoint.x + 25, snapPoint.y),
                wxPoint(snapPoint.x, snapPoint.y + 18),
                wxPoint(snapPoint.x - 20, snapPoint.y + 20)
            };
            dc.DrawSpline(5, rightPoints);//绘制或门右侧部分
            wxPoint leftPoints[3] = {
                wxPoint(snapPoint.x - 20, snapPoint.y - 20),
                wxPoint(snapPoint.x - 10, snapPoint.y),
                wxPoint(snapPoint.x - 20, snapPoint.y + 20),
            };
            dc.DrawSpline(3, leftPoints);//绘制或门左侧部分
            dc.DrawLine(snapPoint.x - 14, snapPoint.y + 10, snapPoint.x - 25, snapPoint.y + 10);
            dc.DrawLine(snapPoint.x - 14, snapPoint.y - 10, snapPoint.x - 25, snapPoint.y - 10);
            dc.DrawLine(snapPoint.x + 20, snapPoint.y, snapPoint.x + 25, snapPoint.y);
        }
        else if (tool == Tool::NOT_GATE) {
            dc.SetPen(wxPen(*wxBLACK, 4)); // 边框颜色和宽度;
            wxPoint points[3] = {
                wxPoint(snapPoint.x - 20, snapPoint.y - 20),
                wxPoint(snapPoint.x + 12, snapPoint.y),
                wxPoint(snapPoint.x - 20, snapPoint.y + 20)
            };
            dc.DrawPolygon(3, points); // 绘制非门左侧部分
            dc.DrawCircle(snapPoint.x + 16, snapPoint.y, 4);
            dc.DrawLine(snapPoint.x - 20, snapPoint.y, snapPoint.x - 27, snapPoint.y);
            dc.DrawLine(snapPoint.x + 20, snapPoint.y, snapPoint.x + 25, snapPoint.y);
        }
    }


    void OnLeftDown(wxMouseEvent& event) {
        wxPoint pos = event.GetPosition(); // 获取鼠标点击位置
        // 检查是否点击在现有组件上
        for (size_t i = 0; i < components.size(); ++i) {
            if (abs(components[i].second.x - pos.x) < 20 && abs(components[i].second.y - pos.y) < 20) {
                dragging = true;
                draggedComponentIndex = i;
                dragStartPos = pos;
                CaptureMouse();
                return;
            }
        }

        // 如果没有拖动组件并且选择了工具，则添加新组件
        if (currentTool != Tool::NONE) {
            components.emplace_back(currentTool, pos); // 添加组件
            Refresh(); // 刷新绘图
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
        // 如果正在拖动组件
        if (dragging) {
            wxPoint pos = event.GetPosition(); // 获取当前鼠标位置
            // 计算偏移量
            wxPoint offset = pos - dragStartPos;
            // 更新组件位置
            components[draggedComponentIndex].second += offset;
            dragStartPos = pos; // 更新拖动开始位置
            Refresh(); // 刷新绘图
        }
    }

    void OnRightDown(wxMouseEvent& event) {
        wxPoint pos = event.GetPosition(); // 获取鼠标点击位置
        bool componentFound = false; // 标记是否找到组件
        int componentToDelete = -1; // 记录要删除的组件索引

        // 检查是否点击在现有组件上
        for (size_t i = 0; i < components.size(); ++i) {
            if (abs(components[i].second.x - pos.x) < 20 && abs(components[i].second.y - pos.y) < 20) {
                componentFound = true; // 找到组件
                componentToDelete = i; // 记录组件索引
                break; // 退出循环
            }
        }

        // 如果找到组件，则显示删除菜单
        if (componentFound) {
            wxMenu menu; // 创建上下文菜单
            menu.Append(wxID_ANY, "Delete"); // 添加删除选项
            // 绑定菜单项的事件
            Bind(wxEVT_MENU, [this, componentToDelete](wxCommandEvent&) {
                if (componentToDelete != -1) {
                    components.erase(components.begin() + componentToDelete); // 删除组件
                    Refresh(); // 刷新绘图
                }
                }, wxID_ANY); // 使用绑定的命令ID
            PopupMenu(&menu); // 显示菜单
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