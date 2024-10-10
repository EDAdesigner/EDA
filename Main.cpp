#include <wx/wx.h>
#include <wx/toolbar.h>
class MyFrame : public wxFrame {
public:
    MyFrame()
        : wxFrame(nullptr, wxID_ANY, "My wxFrame Example") {
        // 创建状态栏
        CreateStatusBar(1);
        SetStatusText("Welcome to wxWidgets!");

        // 创建面板
        wxPanel* panel = new wxPanel(this);
        panel->SetBackgroundColour(*wxLIGHT_GREY);

        // 创建菜单
        wxMenuBar* menuBar = new wxMenuBar;
        wxMenu* fileMenu = new wxMenu;
        fileMenu->Append(wxID_EXIT, "&Exit\tCtrl-Q", "Exit the application");
        menuBar->Append(fileMenu, "&File");
        wxMenu* fileMenu2 = new wxMenu;
        fileMenu2->Append(wxID_APPLY, "&Apply\tCtrl-A", "Apply the change");
        menuBar->Append(fileMenu2, "&Apply");
        SetMenuBar(menuBar);

        // 绑定退出事件
        Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);

        //注册图像处理程序
        wxImage::AddHandler(new wxPNGHandler());

        // 创建按钮以打开子窗口
        wxButton* button = new wxButton(panel, wxID_ANY, "Open Child Window", wxPoint(10, 10));
        wxBitmap myBitmap("image/a.png", wxBITMAP_TYPE_PNG);
        button->SetBitmap(myBitmap);
        button->SetBitmapMargins(5, 5);
        button->Bind(wxEVT_BUTTON, &MyFrame::OnOpenChildWindow, this);

        // 设置大小和显示
        SetSize(800, 600);
        Show(true);

        // 创建竖直工具栏
        wxToolBar* toolbar = CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT);

        // 添加工具
        toolbar->AddTool(wxID_NEW, "New", wxNullBitmap, "Create a new file");
        toolbar->AddTool(wxID_OPEN, "Open", wxNullBitmap, "Open a file");
        toolbar->AddTool(wxID_SAVE, "Save", wxNullBitmap, "Save the file");

        // 实现工具栏的布局
        toolbar->Realize();

        // 绑定工具点击事件
        Bind(wxEVT_TOOL, &MyFrame::OnToolClicked, this, wxID_NEW);
        Bind(wxEVT_TOOL, &MyFrame::OnToolClicked, this, wxID_OPEN);
        Bind(wxEVT_TOOL, &MyFrame::OnToolClicked, this, wxID_SAVE);
    }

private:
    void OnExit(wxCommandEvent& event) {
        Close(true);
    }

    void OnOpenChildWindow(wxCommandEvent& event) {
        wxDialog* childDialog = new wxDialog(this, wxID_ANY, "Child Window", wxDefaultPosition, wxSize(300, 200));
        childDialog->ShowModal(); // 显示模态对话框
        childDialog->Destroy();   // 关闭对话框后销毁
    }
    void OnToolClicked(wxCommandEvent& event) {
        switch (event.GetId()) {
        case wxID_NEW:
            wxLogMessage("New tool clicked!");
            break;
        case wxID_OPEN:
            wxLogMessage("Open tool clicked!");
            break;
        case wxID_SAVE:
            wxLogMessage("Save tool clicked!");
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


wxIMPLEMENT_APP(MyApp);