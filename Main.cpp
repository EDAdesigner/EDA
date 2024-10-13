#include <wx/wx.h>          // 包含 wxWidgets 的核心头文件
#include <wx/toolbar.h>     // 包含工具栏的相关功能
#include <wx/filedlg.h>     // 包含文件对话框的功能
#include <wx/image.h>       // 包含图像处理的功能
#include <wx/artprov.h>     // 包含工具栏图标的功能

// 定义主框架类 MyFrame，继承自 wxFrame
class MyFrame : public wxFrame {
public:
    MyFrame()  // 构造函数
        : wxFrame(nullptr, wxID_ANY, "EDA Example") { // 初始化父类 wxFrame

        // 创建面板
        wxPanel* panel = new wxPanel(this);  // 创建面板，作为主窗口的子窗口
        panel->SetBackgroundColour(*wxLIGHT_GREY);  // 设置面板背景颜色

        // 创建水平布局管理器
        wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);

        // 创建子面板
        wxPanel* subPanel = new wxPanel(panel, wxID_ANY);
        subPanel->SetBackgroundColour(*wxLIGHT_GREY);
        hbox->Add(subPanel, 2, wxEXPAND | wxALL, 10);

        // 创建画板
        wxPanel* drawPanel = new wxPanel(panel, wxID_ANY);
        drawPanel->SetBackgroundColour(*wxWHITE);
        hbox->Add(drawPanel, 8, wxEXPAND | wxALL, 10);
        //设置水平布局管理器
        panel->SetSizer(hbox);

        // 创建状态栏
        CreateStatusBar(1);  // 创建状态栏，显示一行
        SetStatusText("This is a model");  // 设置状态栏文本

        // 创建菜单
        wxMenuBar* menuBar = new wxMenuBar;  // 创建菜单栏
        wxMenu* fileMenu = new wxMenu;  // 创建文件菜单
        // 向文件菜单添加项
        fileMenu->Append(wxID_NEW, "&New\tCtrl-N", "Create a new file");
        fileMenu->Append(wxID_OPEN, "&Open\tCtrl-O", "Open a file");
        fileMenu->Append(wxID_SAVE, "&Save\tCtrl-S", "Save the file");
        fileMenu->AppendSeparator();  // 添加分隔符
        fileMenu->Append(wxID_EXIT, "&Exit\tCtrl-Q", "Exit the application");
        menuBar->Append(fileMenu, "&File");  // 将文件菜单添加到菜单栏

        // 创建帮助菜单
        wxMenu* helpMenu = new wxMenu;
        helpMenu->Append(wxID_ABOUT, "&About\tF1", "Show about dialog");
        menuBar->Append(helpMenu, "&Help");  // 将帮助菜单添加到菜单栏

        SetMenuBar(menuBar);  // 设置菜单栏

        // 设置窗口大小并显示
        SetSize(800, 600);
        Show(true);  // 显示窗口

        // 创建水平工具栏
        wxToolBar* toolbar = CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT);
        // 添加工具栏项
        auto toolNew = toolbar->AddTool(wxID_NEW, "New", wxArtProvider::GetBitmap(wxART_NEW));
        auto toolOpen = toolbar->AddTool(wxID_OPEN, "Open", wxArtProvider::GetBitmap(wxART_FILE_OPEN));
        auto toolSave = toolbar->AddTool(wxID_SAVE, "Save", wxArtProvider::GetBitmap(wxART_FILE_SAVE));

        //实现工具栏
        toolbar->Realize();

        //基于侧面面板创建侧面工具栏
        wxToolBar* subtoolbar = new wxToolBar(subPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxNO_BORDER);
        //添加工具栏项
        auto subtoolNew = subtoolbar->AddTool(wxID_NEW, "New", wxArtProvider::GetBitmap(wxART_NEW));
        auto subtoolOpen = subtoolbar->AddTool(wxID_OPEN, "Open", wxArtProvider::GetBitmap(wxART_FILE_OPEN));
        auto subtoolSave = subtoolbar->AddTool(wxID_SAVE, "Save", wxArtProvider::GetBitmap(wxART_FILE_SAVE));


        // 实现工具栏
        subtoolbar->Realize();
        // 设置工具栏大小和位置，使其填满子面板
        subtoolbar->SetSize(subPanel->GetClientSize());
        subtoolbar->SetPosition(wxPoint(0, 0));
        // 绑定子面板大小变化事件，以便在子面板大小变化时调整工具栏大小
        subPanel->Bind(wxEVT_SIZE, [subtoolbar](wxSizeEvent& event) {
            subtoolbar->SetSize(event.GetSize());
            event.Skip();
            });

        // 绑定事件到相应的处理函数
        Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);  // 绑定退出事件
        Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);  // 绑定关于事件
        Bind(wxEVT_MENU, &MyFrame::OnNew, this, wxID_NEW);  // 绑定新建文件事件
        Bind(wxEVT_MENU, &MyFrame::OnOpen, this, wxID_OPEN);  // 绑定打开文件事件
        Bind(wxEVT_MENU, &MyFrame::OnSave, this, wxID_SAVE);  // 绑定保存文件事件
    }

private:
    // 退出事件处理函数
    void OnExit(wxCommandEvent& event) {
        Close(true);  // 关闭窗口
    }

    // 关于事件处理函数
    void OnAbout(wxCommandEvent& event) {
        wxMessageBox("This is a wxWidgets EDA application.", "About My Application", wxOK | wxICON_INFORMATION);  // 显示关于对话框
    }

    // 新建文件事件处理函数
    void OnNew(wxCommandEvent& event) {
        wxLogMessage("New file created!");  // 日志输出新建文件消息
    }

    // 打开文件事件处理函数
    void OnOpen(wxCommandEvent& event) {
        // 创建文件打开对话框
        wxFileDialog openFileDialog(this, "Open File", "", "", "All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (openFileDialog.ShowModal() == wxID_OK) {  // 如果用户选择了文件
            wxString path = openFileDialog.GetPath();  // 获取文件路径
            wxLogMessage("Opened file: %s", path);  // 日志输出打开的文件路径
        }
    }

    // 保存文件事件处理函数
    void OnSave(wxCommandEvent& event) {
        // 创建文件保存对话框
        wxFileDialog saveFileDialog(this, "Save File", "", "", "All files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (saveFileDialog.ShowModal() == wxID_OK) {  // 如果用户选择了文件
            wxString path = saveFileDialog.GetPath();  // 获取文件路径
            wxLogMessage("Saved file: %s", path);  // 日志输出保存的文件路径
        }
    }


    // 工具栏点击事件处理函数
    void OnToolClicked(wxCommandEvent& event) {
        switch (event.GetId()) {  // 根据事件 ID 选择处理函数
        case wxID_NEW:
            OnNew(event);  // 调用新建文件处理函数
            break;
        case wxID_OPEN:
            OnOpen(event);  // 调用打开文件处理函数
            break;
        case wxID_SAVE:
            OnSave(event);  // 调用保存文件处理函数
            break;
        }
    }
};

// 定义应用程序类 MyApp，继承自 wxApp
class MyApp : public wxApp {
public:
    virtual bool OnInit() {  // 应用程序初始化函数
        MyFrame* frame = new MyFrame();  // 创建主窗口
        return true;  // 返回 true 表示初始化成功
    }
};

// 实现 wxWidgets 应用程序
wxIMPLEMENT_APP(MyApp);
