#include <wx/wx.h>
#include <wx/toolbar.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/artprov.h>
#include <vector>

// 主框架类，继承自wxFrame
class MyFrame : public wxFrame {
public:
    // 构造函数
    MyFrame()
        : wxFrame(nullptr, wxID_ANY, "EDA Example") {

        // 创建主面板，作为应用程序的基本界面
        wxPanel* panel = new wxPanel(this);
        panel->SetBackgroundColour(*wxLIGHT_GREY); // 设置主面板的背景颜色为浅灰色

        // 创建水平布局管理器，用于管理子面板和绘图面板的布局
        wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);

        // 创建子面板，作为工具栏和其他控件的容器
        wxPanel* subPanel = new wxPanel(panel, wxID_ANY);
        subPanel->SetBackgroundColour(*wxLIGHT_GREY); // 设置子面板的背景颜色为浅灰色
        hbox->Add(subPanel, 2, wxEXPAND | wxALL, 10); // 将子面板添加到布局中，比例为2，允许扩展，并添加边距

        // 创建绘图面板，用于显示和绘制电子元件
        drawPanel = new DrawPanel(panel);
        hbox->Add(drawPanel, 8, wxEXPAND | wxALL, 10); // 将绘图面板添加到布局中，比例为8，允许扩展，并添加边距

        // 设置主面板的布局管理器为hbox
        panel->SetSizer(hbox);

        // 创建状态栏，显示应用程序状态
        CreateStatusBar(1);
        SetStatusText("This is a model"); // 设置状态栏的文本

        // 创建菜单栏，包含文件和帮助菜单
        wxMenuBar* menuBar = new wxMenuBar;
        wxMenu* fileMenu = new wxMenu; // 创建文件菜单
        fileMenu->Append(wxID_NEW, "&New\tCtrl-N", "Create a new file"); // 添加新建文件选项
        fileMenu->Append(wxID_OPEN, "&Open\tCtrl-O", "Open a file"); // 添加打开文件选项
        fileMenu->Append(wxID_SAVE, "&Save\tCtrl-S", "Save the file"); // 添加保存文件选项
        fileMenu->AppendSeparator(); // 添加分隔符
        fileMenu->Append(wxID_EXIT, "&Exit\tCtrl-Q", "Exit the application"); // 添加退出应用选项
        menuBar->Append(fileMenu, "&File"); // 将文件菜单添加到菜单栏

        wxMenu* helpMenu = new wxMenu; // 创建帮助菜单
        helpMenu->Append(wxID_ABOUT, "&About\tF1", "Show about dialog"); // 添加关于选项
        menuBar->Append(helpMenu, "&Help"); // 将帮助菜单添加到菜单栏

        SetMenuBar(menuBar); // 设置应用程序的菜单栏
        SetSize(800, 600); // 设置窗口的初始大小
        Show(true); // 显示窗口

        // 创建工具栏，用于快速访问功能
        wxToolBar* toolbar = CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT); // 创建水平工具栏
        toolbar->AddTool(wxID_NEW, "New", wxArtProvider::GetBitmap(wxART_NEW)); // 添加新建工具图标
        toolbar->AddTool(wxID_OPEN, "Open", wxArtProvider::GetBitmap(wxART_FILE_OPEN)); // 添加打开工具图标
        toolbar->AddTool(wxID_SAVE, "Save", wxArtProvider::GetBitmap(wxART_FILE_SAVE)); // 添加保存工具图标
        toolbar->Realize(); // 完成工具栏的创建

        // 创建子工具栏，用于选择不同的电子元件
        wxToolBar* subtoolbar = new wxToolBar(subPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxNO_BORDER);
        subtoolbar->AddTool(1, "AND Gate", wxArtProvider::GetBitmap(wxART_NEW)); // 添加与门图标
        subtoolbar->AddTool(2, "OR Gate", wxArtProvider::GetBitmap(wxART_NEW)); // 添加或门图标
        subtoolbar->AddTool(3, "NOT Gate", wxArtProvider::GetBitmap(wxART_NEW)); // 添加非门图标
        subtoolbar->AddTool(4, "Delete", wxArtProvider::GetBitmap(wxART_NEW)); // 添加删除工具图标
        subtoolbar->Realize(); // 完成子工具栏的创建

        // 设置子工具栏的大小和位置
        subtoolbar->SetSize(subPanel->GetClientSize()); // 将子工具栏的大小设置为子面板的客户区大小
        subtoolbar->SetPosition(wxPoint(0, 0)); // 设置子工具栏的位置为(0, 0)

        // 绑定子面板大小变化事件，确保子工具栏在大小变化时也跟随调整
        subPanel->Bind(wxEVT_SIZE, [subtoolbar](wxSizeEvent& event) {
            subtoolbar->SetSize(event.GetSize()); // 更新子工具栏的大小
            event.Skip(); // 继续处理事件
            });

        // 绑定菜单事件，响应用户的菜单操作
        Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT); // 绑定退出事件
        Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT); // 绑定关于事件
        Bind(wxEVT_MENU, &MyFrame::OnNew, this, wxID_NEW); // 绑定新建事件
        Bind(wxEVT_MENU, &MyFrame::OnOpen, this, wxID_OPEN); // 绑定打开事件
        Bind(wxEVT_MENU, &MyFrame::OnSave, this, wxID_SAVE); // 绑定保存事件

        // 绑定子工具栏事件，响应工具选择
        subtoolbar->Bind(wxEVT_TOOL, &MyFrame::OnSelectTool, this); // 绑定工具选择事件

    }

private:
    class DrawPanel : public wxPanel {
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

    private:
        Tool currentTool; // 当前工具
        std::vector<std::pair<Tool, wxPoint>> components; // 存储已添加的组件及其位置
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
                wxPoint points[5] = {
                    wxPoint(snapPoint.x - 20, snapPoint.y - 20), // 左上
                    wxPoint(snapPoint.x , snapPoint.y - 20), // 右上
                    wxPoint(snapPoint.x , snapPoint.y + 20), // 中下
                    wxPoint(snapPoint.x - 20, snapPoint.y + 20)  // 左下
                };
                dc.SetPen(wxPen(*wxBLACK, 4)); // 边框颜色和宽度
                dc.DrawPolygon(4, points); // 绘制与门
                dc.DrawArc(snapPoint.x, snapPoint.y + 20, snapPoint.x, snapPoint.y - 20, snapPoint.x, snapPoint.y); // 绘制圆边
            }
            else if (tool == Tool::OR_GATE) {
                dc.SetBrush(*wxYELLOW_BRUSH);
                dc.DrawEllipse(snapPoint.x - 15, snapPoint.y - 10, 30, 20);
            }
            else if (tool == Tool::NOT_GATE) {
                dc.SetBrush(*wxRED_BRUSH);
                dc.DrawRectangle(snapPoint.x - 10, snapPoint.y - 10, 20, 20);
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
    };


    // 声明绘图面板指针，用于操作绘制的组件
    DrawPanel* drawPanel;

    // 处理退出事件
    void OnExit(wxCommandEvent& event) {
        Close(true); // 关闭应用程序窗口
    }

    // 处理关于对话框事件
    void OnAbout(wxCommandEvent& event) {
        // 显示关于信息的对话框
        wxMessageBox("This is a wxWidgets EDA application.", "About My Application", wxOK | wxICON_INFORMATION);
    }

    // 处理新建文件事件
    void OnNew(wxCommandEvent& event) {
        drawPanel->Refresh(); // 刷新绘图面板以清空内容
        wxLogMessage("New file created!"); // 在日志中记录新建文件的操作
    }

    // 处理打开文件事件
    void OnOpen(wxCommandEvent& event) {
        // 创建文件对话框，允许用户选择要打开的文件
        wxFileDialog openFileDialog(this, "Open File", "", "", "All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (openFileDialog.ShowModal() == wxID_OK) { // 显示对话框并检查用户是否选择了文件
            wxString path = openFileDialog.GetPath(); // 获取选定文件的路径
            wxLogMessage("Opened file: %s", path); // 在日志中记录打开文件的路径
        }
    }

    // 处理保存文件事件
    void OnSave(wxCommandEvent& event) {
        // 创建文件对话框，允许用户选择保存的文件位置和名称
        wxFileDialog saveFileDialog(this, "Save File", "", "", "All files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (saveFileDialog.ShowModal() == wxID_OK) { // 显示对话框并检查用户是否选择了文件
            wxString path = saveFileDialog.GetPath(); // 获取用户选择的文件路径
            wxLogMessage("Saved file: %s", path); // 在日志中记录保存文件的路径
        }
    }

    // 处理选择工具事件
    void OnSelectTool(wxCommandEvent& event) {
        int toolId = event.GetId(); // 获取被选择工具的ID
        switch (toolId) { // 根据工具ID选择相应的工具
        case 1: // 选择与门工具
            drawPanel->SetCurrentTool(DrawPanel::Tool::AND_GATE);
            break;
        case 2: // 选择或门工具
            drawPanel->SetCurrentTool(DrawPanel::Tool::OR_GATE);
            break;
        case 3: // 选择非门工具
            drawPanel->SetCurrentTool(DrawPanel::Tool::NOT_GATE);
            break;
        case 4: // 删除工具逻辑不再需要，因为使用右键菜单删除
            // 这里可以选择什么也不做
            break;
        }
    }
};

class MyApp : public wxApp {
public:
    virtual bool OnInit() {
        MyFrame* frame = new MyFrame();
        return true;
    }
};

// 实现应用程序入口点
wxIMPLEMENT_APP(MyApp);