#include <wx/wx.h>          // 包含 wxWidgets 的核心头文件
#include <wx/toolbar.h>     // 包含工具栏的相关功能
#include <wx/filedlg.h>     // 包含文件对话框的功能
#include <wx/image.h>       // 包含图像处理的功能

// 定义主框架类 MyFrame，继承自 wxFrame
class MyFrame : public wxFrame {
public:
    MyFrame()  // 构造函数
        : wxFrame(nullptr, wxID_ANY, "My wxFrame Example") { // 初始化父类 wxFrame
        // 创建状态栏
        CreateStatusBar(1);  // 创建状态栏，显示一行
        SetStatusText("Welcome to wxWidgets!");  // 设置状态栏文本

        // 创建面板
        wxPanel* panel = new wxPanel(this);  // 创建面板，作为主窗口的子窗口
        panel->SetBackgroundColour(*wxLIGHT_GREY);  // 设置面板背景颜色

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

        // 绑定菜单事件到相应的处理函数
        Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);  // 绑定退出事件
        Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);  // 绑定关于事件
        Bind(wxEVT_MENU, &MyFrame::OnNew, this, wxID_NEW);  // 绑定新建文件事件
        Bind(wxEVT_MENU, &MyFrame::OnOpen, this, wxID_OPEN);  // 绑定打开文件事件
        Bind(wxEVT_MENU, &MyFrame::OnSave, this, wxID_SAVE);  // 绑定保存文件事件

        // 注册图像处理程序（支持 PNG 格式）
        wxImage::AddHandler(new wxPNGHandler());

        // 创建按钮以打开子窗口
        wxButton* button = new wxButton(panel, wxID_ANY, "Open Child Window", wxPoint(10, 10));
        // wxBitmap myBitmap("image/a.png", wxBITMAP_TYPE_PNG); // 可选：设置按钮图标
        // button->SetBitmap(myBitmap);
        // button->SetBitmapMargins(5, 5);
        button->Bind(wxEVT_BUTTON, &MyFrame::OnOpenChildWindow, this);  // 绑定按钮点击事件

        // 设置窗口大小并显示
        SetSize(800, 600);
        Show(true);  // 显示窗口

        // 创建水平工具栏
        wxToolBar* toolbar = CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT);
        // 向工具栏添加工具
        toolbar->AddTool(wxID_NEW, "New", wxNullBitmap, "Create a new file");
        toolbar->AddTool(wxID_OPEN, "Open", wxNullBitmap, "Open a file");
        toolbar->AddTool(wxID_SAVE, "Save", wxNullBitmap, "Save the file");
        toolbar->Realize();  // 实现工具栏

        // 绑定工具栏点击事件
        Bind(wxEVT_TOOL, &MyFrame::OnToolClicked, this, wxID_NEW);
        Bind(wxEVT_TOOL, &MyFrame::OnToolClicked, this, wxID_OPEN);
        Bind(wxEVT_TOOL, &MyFrame::OnToolClicked, this, wxID_SAVE);
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

    // 打开子窗口事件处理函数
    void OnOpenChildWindow(wxCommandEvent& event) {
        wxDialog* childDialog = new wxDialog(this, wxID_ANY, "Child Window", wxDefaultPosition, wxSize(300, 200));  // 创建子窗口
        childDialog->ShowModal();  // 显示模态对话框
        childDialog->Destroy();  // 关闭对话框后销毁
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
