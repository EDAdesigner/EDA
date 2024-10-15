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
        : wxFrame(nullptr, wxID_ANY, "EDA Example") { // 调用基类构造函数，设置标题

        // 创建主面板
        wxPanel* panel = new wxPanel(this);
        panel->SetBackgroundColour(*wxLIGHT_GREY); // 设置背景颜色为浅灰色

        // 创建水平布局管理器
        wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);

        // 创建子面板
        wxPanel* subPanel = new wxPanel(panel, wxID_ANY);
        subPanel->SetBackgroundColour(*wxLIGHT_GREY); // 设置子面板的背景颜色
        hbox->Add(subPanel, 2, wxEXPAND | wxALL, 10); // 将子面板添加到布局管理器中，权重为2

        // 创建绘图面板
        drawPanel = new DrawPanel(panel);
        hbox->Add(drawPanel, 8, wxEXPAND | wxALL, 10); // 将绘图面板添加到布局管理器中，权重为8

        panel->SetSizer(hbox); // 设置主面板的布局管理器

        CreateStatusBar(1); // 创建状态栏
        SetStatusText("This is a model"); // 设置状态栏文本

        // 创建菜单栏
        wxMenuBar* menuBar = new wxMenuBar;
        wxMenu* fileMenu = new wxMenu; // 文件菜单
        fileMenu->Append(wxID_NEW, "&New\tCtrl-N", "Create a new file"); // 新建
        fileMenu->Append(wxID_OPEN, "&Open\tCtrl-O", "Open a file"); // 打开
        fileMenu->Append(wxID_SAVE, "&Save\tCtrl-S", "Save the file"); // 保存
        fileMenu->AppendSeparator(); // 菜单分隔线
        fileMenu->Append(wxID_EXIT, "&Exit\tCtrl-Q", "Exit the application"); // 退出
        menuBar->Append(fileMenu, "&File"); // 添加文件菜单到菜单栏

        wxMenu* helpMenu = new wxMenu; // 帮助菜单
        helpMenu->Append(wxID_ABOUT, "&About\tF1", "Show about dialog"); // 关于
        menuBar->Append(helpMenu, "&Help"); // 添加帮助菜单到菜单栏

        SetMenuBar(menuBar); // 设置菜单栏
        SetSize(800, 600); // 设置窗口大小
        Show(true); // 显示窗口

        // 创建工具栏
        wxToolBar* toolbar = CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT);
        toolbar->AddTool(wxID_NEW, "New", wxArtProvider::GetBitmap(wxART_NEW)); // 新建工具
        toolbar->AddTool(wxID_OPEN, "Open", wxArtProvider::GetBitmap(wxART_FILE_OPEN)); // 打开工具
        toolbar->AddTool(wxID_SAVE, "Save", wxArtProvider::GetBitmap(wxART_FILE_SAVE)); // 保存工具
        toolbar->Realize(); // 确认工具栏的设置

        // 创建子工具栏
        wxToolBar* subtoolbar = new wxToolBar(subPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxNO_BORDER);
        subtoolbar->AddTool(wxID_NEW, "New", wxArtProvider::GetBitmap(wxART_NEW)); // 新建工具
        subtoolbar->AddTool(wxID_OPEN, "Open", wxArtProvider::GetBitmap(wxART_FILE_OPEN)); // 打开工具
        subtoolbar->AddTool(wxID_SAVE, "Save", wxArtProvider::GetBitmap(wxART_FILE_SAVE)); // 保存工具
        subtoolbar->Realize(); // 确认子工具栏的设置

        // 设置子工具栏的大小和位置
        subtoolbar->SetSize(subPanel->GetClientSize());
        subtoolbar->SetPosition(wxPoint(0, 0));

        // 绑定子面板大小变化事件
        subPanel->Bind(wxEVT_SIZE, [subtoolbar](wxSizeEvent& event) {
            subtoolbar->SetSize(event.GetSize()); // 设置子工具栏大小
            event.Skip(); // 继续处理其他事件
            });

        // 绑定菜单事件
        Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT); // 退出事件
        Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT); // 关于事件
        Bind(wxEVT_MENU, &MyFrame::OnNew, this, wxID_NEW); // 新建事件
        Bind(wxEVT_MENU, &MyFrame::OnOpen, this, wxID_OPEN); // 打开事件
        Bind(wxEVT_MENU, &MyFrame::OnSave, this, wxID_SAVE); // 保存事件
    }


private:
    class DrawPanel : public wxPanel {
    public:
        enum class Tool { NONE, AND_GATE, OR_GATE, NOT_GATE, DELETE_GATE }; // 添加 DELETE_GATE

        DrawPanel(wxWindow* parent)
            : wxPanel(parent), currentTool(Tool::NONE) {
            SetBackgroundColour(*wxWHITE);
            Bind(wxEVT_PAINT, &DrawPanel::OnPaint, this);
            Bind(wxEVT_LEFT_DOWN, &DrawPanel::OnLeftDown, this);
            Bind(wxEVT_RIGHT_DOWN, &DrawPanel::OnRightDown, this);
            Bind(wxEVT_SIZE, &DrawPanel::OnSize, this);
        }

        void SetCurrentTool(Tool tool) {
            currentTool = tool;
        }

    private:
        // 当前选择的工具
        Tool currentTool;

        // 存储组件及其位置的向量
        std::vector<std::pair<Tool, wxPoint>> components;

        // 处理绘制事件
        void OnPaint(wxPaintEvent& event) {
            wxPaintDC dc(this); // 创建一个绘图上下文
            DrawGrid(dc); // 绘制网格
            // 遍历所有组件并绘制它们
            for (const auto& component : components) {
                DrawComponent(dc, component.first, component.second);
            }
        }

        // 绘制网格
        void DrawGrid(wxDC& dc) {
            dc.SetPen(*wxLIGHT_GREY_PEN); // 设置画笔为浅灰色
            // 绘制垂直线
            for (int i = 0; i < GetSize().GetWidth(); i += 20) {
                dc.DrawLine(i, 0, i, GetSize().GetHeight());
            }
            // 绘制水平线
            for (int j = 0; j < GetSize().GetHeight(); j += 20) {
                dc.DrawLine(0, j, GetSize().GetWidth(), j);
            }
        }

        // 根据工具类型绘制组件
        void DrawComponent(wxDC& dc, Tool tool, const wxPoint& pos) {
            if (tool == Tool::AND_GATE) {
                dc.SetBrush(*wxGREEN_BRUSH); // 设置画刷为绿色
                dc.DrawRectangle(pos.x - 10, pos.y - 10, 20, 20); // 绘制AND门
            }
            else if (tool == Tool::OR_GATE) {
                dc.SetBrush(*wxYELLOW_BRUSH); // 设置画刷为黄色
                dc.DrawEllipse(pos.x - 15, pos.y - 10, 30, 20); // 绘制OR门
            }
            else if (tool == Tool::NOT_GATE) {
                dc.SetBrush(*wxRED_BRUSH); // 设置画刷为红色
                dc.DrawRectangle(pos.x - 10, pos.y - 10, 20, 20); // 绘制NOT门
            }
        }

        // 处理左键按下事件
        void OnLeftDown(wxMouseEvent& event) {
            wxPoint pos = event.GetPosition(); // 获取鼠标点击位置
            if (currentTool != Tool::NONE) { // 如果选择了有效的工具
                if (currentTool == Tool::DELETE_GATE) {
                    // 查找并删除组件
                    for (auto it = components.begin(); it != components.end(); ) {
                        // 检查鼠标位置是否在组件附近
                        if (abs(it->second.x - pos.x) < 20 && abs(it->second.y - pos.y) < 20) {
                            it = components.erase(it); // 删除组件
                            Refresh(); // 刷新面板以重新绘制
                            return; // 只删除一个组件
                        }
                        else {
                            ++it; // 移动到下一个组件
                        }
                    }
                }
                else {
                    components.emplace_back(currentTool, pos); // 添加新组件
                    Refresh(); // 刷新面板以重新绘制
                }
            }
        }

        // 处理右键按下事件
        void OnRightDown(wxMouseEvent& event) {
            // 切换工具
            if (currentTool == Tool::NONE) {
                currentTool = Tool::AND_GATE; // 选择AND门
            }
            else if (currentTool == Tool::AND_GATE) {
                currentTool = Tool::OR_GATE; // 选择OR门
            }
            else if (currentTool == Tool::OR_GATE) {
                currentTool = Tool::NOT_GATE; // 选择NOT门
            }
            else if (currentTool == Tool::NOT_GATE) {
                currentTool = Tool::DELETE_GATE; // 切换到删除工具
            }
            else {
                currentTool = Tool::NONE; // 切换回无工具
            }

            // 显示当前选择的工具
            wxString toolName = (currentTool == Tool::AND_GATE) ? "AND Gate" :
                (currentTool == Tool::OR_GATE) ? "OR Gate" :
                (currentTool == Tool::NOT_GATE) ? "NOT Gate" :
                (currentTool == Tool::DELETE_GATE) ? "Delete Gate" : "None";

            wxLogMessage("Current Tool: %s", toolName); // 记录当前工具
        }

        // 处理窗口大小变化事件
        void OnSize(wxSizeEvent& event) {
            Refresh(); // 刷新面板以重新绘制内容
            event.Skip(); // 继续处理其他大小变化事件
        }
    };


    DrawPanel* drawPanel;

    // 处理退出事件
    void OnExit(wxCommandEvent& event) {
        Close(true); // 关闭当前窗口并退出应用程序
    }

    // 处理关于事件
    void OnAbout(wxCommandEvent& event) {
        // 显示关于对话框，包含应用程序的描述信息
        wxMessageBox("This is a wxWidgets EDA application.", "About My Application", wxOK | wxICON_INFORMATION);
    }

    // 处理新建文件事件
    void OnNew(wxCommandEvent& event) {
        // 清空绘图面板（可以解除注释以实现）
        // drawPanel->Refresh();
        wxLogMessage("New file created!"); // 记录新建文件的消息
    }

    // 处理打开文件事件
    void OnOpen(wxCommandEvent& event) {
        // 创建文件对话框以选择要打开的文件
        wxFileDialog openFileDialog(this, "Open File", "", "", "All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (openFileDialog.ShowModal() == wxID_OK) { // 显示对话框并检查用户是否点击了“OK”
            wxString path = openFileDialog.GetPath(); // 获取选择的文件路径
            wxLogMessage("Opened file: %s", path); // 记录打开文件的消息
        }
    }

    // 处理保存文件事件
    void OnSave(wxCommandEvent& event) {
        // 创建文件对话框以选择保存的文件位置
        wxFileDialog saveFileDialog(this, "Save File", "", "", "All files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (saveFileDialog.ShowModal() == wxID_OK) { // 显示对话框并检查用户是否点击了“OK”
            wxString path = saveFileDialog.GetPath(); // 获取选择的文件路径
            wxLogMessage("Saved file: %s", path); // 记录保存文件的消息
        }
    };
};

class MyApp : public wxApp {
public:
    // 初始化应用程序
    virtual bool OnInit() {
        MyFrame* frame = new MyFrame(); // 创建主框架
        return true; // 返回成功
    }
};

// 实现应用程序入口点
wxIMPLEMENT_APP(MyApp);