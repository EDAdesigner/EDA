#include <wx/wx.h>
#include <wx/toolbar.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/artprov.h>
#include <vector>

class MyFrame : public wxFrame {
public:
    MyFrame()
        : wxFrame(nullptr, wxID_ANY, "EDA Example") {
        // 创建状态栏
        wxPanel* panel = new wxPanel(this);
        panel->SetBackgroundColour(*wxLIGHT_GREY);
        // 创建面板
        wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);

        wxPanel* subPanel = new wxPanel(panel, wxID_ANY);
        subPanel->SetBackgroundColour(*wxLIGHT_GREY);
        hbox->Add(subPanel, 2, wxEXPAND | wxALL, 10);

        drawPanel = new DrawPanel(panel);
        hbox->Add(drawPanel, 8, wxEXPAND | wxALL, 10);
        panel->SetSizer(hbox);

        CreateStatusBar(1);
        SetStatusText("This is a model");

        wxMenuBar* menuBar = new wxMenuBar;
        wxMenu* fileMenu = new wxMenu;
        fileMenu->Append(wxID_NEW, "&New\tCtrl-N", "Create a new file");
        fileMenu->Append(wxID_OPEN, "&Open\tCtrl-O", "Open a file");
        fileMenu->Append(wxID_SAVE, "&Save\tCtrl-S", "Save the file");
        fileMenu->AppendSeparator();
        // 创建菜单
        menuBar->Append(fileMenu, "&File");

        wxMenu* helpMenu = new wxMenu;
        helpMenu->Append(wxID_ABOUT, "&About\tF1", "Show about dialog");
        menuBar->Append(helpMenu, "&Help");

        SetMenuBar(menuBar);
        SetSize(800, 600);
        Show(true);

        wxToolBar* toolbar = CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT);
        toolbar->AddTool(wxID_NEW, "New", wxArtProvider::GetBitmap(wxART_NEW));
        toolbar->AddTool(wxID_OPEN, "Open", wxArtProvider::GetBitmap(wxART_FILE_OPEN));
        toolbar->AddTool(wxID_SAVE, "Save", wxArtProvider::GetBitmap(wxART_FILE_SAVE));
        toolbar->Realize();

        wxToolBar* subtoolbar = new wxToolBar(subPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxNO_BORDER);
        subtoolbar->AddTool(wxID_NEW, "New", wxArtProvider::GetBitmap(wxART_NEW));
        subtoolbar->AddTool(wxID_OPEN, "Open", wxArtProvider::GetBitmap(wxART_FILE_OPEN));
        subtoolbar->AddTool(wxID_SAVE, "Save", wxArtProvider::GetBitmap(wxART_FILE_SAVE));
        subtoolbar->Realize();
        subtoolbar->SetSize(subPanel->GetClientSize());
        subtoolbar->SetPosition(wxPoint(0, 0));
        subPanel->Bind(wxEVT_SIZE, [subtoolbar](wxSizeEvent& event) {
            subtoolbar->SetSize(event.GetSize());
            event.Skip();
            });

        Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
        Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
        Bind(wxEVT_MENU, &MyFrame::OnNew, this, wxID_NEW);
        Bind(wxEVT_MENU, &MyFrame::OnOpen, this, wxID_OPEN);
        Bind(wxEVT_MENU, &MyFrame::OnSave, this, wxID_SAVE);
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
        Tool currentTool;
        std::vector<std::pair<Tool, wxPoint>> components;

        void OnPaint(wxPaintEvent& event) {
            wxPaintDC dc(this);
            DrawGrid(dc);
            for (const auto& component : components) {
                DrawComponent(dc, component.first, component.second);
            }
        }

        void DrawGrid(wxDC& dc) {
            dc.SetPen(*wxLIGHT_GREY_PEN);
            for (int i = 0; i < GetSize().GetWidth(); i += 20) {
                dc.DrawLine(i, 0, i, GetSize().GetHeight());
            }
            for (int j = 0; j < GetSize().GetHeight(); j += 20) {
                dc.DrawLine(0, j, GetSize().GetWidth(), j);
            }
        }

        void DrawComponent(wxDC& dc, Tool tool, const wxPoint& pos) {
            if (tool == Tool::AND_GATE) {
                dc.SetBrush(*wxGREEN_BRUSH);
                dc.DrawRectangle(pos.x - 10, pos.y - 10, 20, 20);
            }
            else if (tool == Tool::OR_GATE) {
                dc.SetBrush(*wxYELLOW_BRUSH);
                dc.DrawEllipse(pos.x - 15, pos.y - 10, 30, 20);
            }
            else if (tool == Tool::NOT_GATE) {
                dc.SetBrush(*wxRED_BRUSH);
                dc.DrawRectangle(pos.x - 10, pos.y - 10, 20, 20);
            }
        }
        button->SetBitmap(myBitmap);
        void OnLeftDown(wxMouseEvent& event) {
            wxPoint pos = event.GetPosition();
            if (currentTool != Tool::NONE) {
                if (currentTool == Tool::DELETE_GATE) {
                    // 查找并删除组件
                    for (auto it = components.begin(); it != components.end(); ) {
                        if (abs(it->second.x - pos.x) < 20 && abs(it->second.y - pos.y) < 20) {
                            it = components.erase(it); // 删除组件
                            Refresh(); // 刷新面板
                            return; // 只删除一个
                        }
                        else {
                            ++it;
                        }
                    }
                }
                else {
                    components.emplace_back(currentTool, pos); // 添加新组件
                    Refresh(); // 刷新面板以重新绘制
                }
            }
        }
        // 设置大小和显示
        void OnRightDown(wxMouseEvent& event) {
            // 切换工具
            if (currentTool == Tool::NONE) {
                currentTool = Tool::AND_GATE;
            }
            else if (currentTool == Tool::AND_GATE) {
                currentTool = Tool::OR_GATE;
            }
            else if (currentTool == Tool::OR_GATE) {
                currentTool = Tool::NOT_GATE;
            }
            else if (currentTool == Tool::NOT_GATE) {
                currentTool = Tool::DELETE_GATE; // 切换到删除工具
            }
            else {
                currentTool = Tool::NONE; // 切换回无工具
            }
        // 设置大小和显示
            wxString toolName = (currentTool == Tool::AND_GATE) ? "AND Gate" :
                (currentTool == Tool::OR_GATE) ? "OR Gate" :
                (currentTool == Tool::NOT_GATE) ? "NOT Gate" :
                (currentTool == Tool::DELETE_GATE) ? "Delete Gate" : "None";
        toolbar->AddTool(wxID_NEW, "New", wxNullBitmap, "Create a new file");
            wxLogMessage("Current Tool: %s", toolName);
        }
        toolbar->AddTool(wxID_NEW, "New", wxNullBitmap, "Create a new file");
        void OnSize(wxSizeEvent& event) {
            Refresh();
            event.Skip();
        }
    };


    DrawPanel* drawPanel;
        Bind(wxEVT_TOOL, &MyFrame::OnToolClicked, this, wxID_OPEN);
    }
        Bind(wxEVT_TOOL, &MyFrame::OnToolClicked, this, wxID_OPEN);
private:
    }

private:
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
    void OnAbout(wxCommandEvent& event) {
        wxMessageBox("This is a wxWidgets EDA application.", "About My Application", wxOK | wxICON_INFORMATION);
    }

    void OnNew(wxCommandEvent& event) {
        //drawPanel->Refresh();
        wxLogMessage("New file created!");
    }

    void OnOpen(wxCommandEvent& event) {
        wxFileDialog openFileDialog(this, "Open File", "", "", "All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (openFileDialog.ShowModal() == wxID_OK) {
            wxString path = openFileDialog.GetPath();
            wxLogMessage("Opened file: %s", path);
        }
        wxDialog* childDialog = new wxDialog(this, wxID_ANY, "Child Window", wxDefaultPosition, wxSize(300, 200));

    void OnSave(wxCommandEvent& event) {
        wxFileDialog saveFileDialog(this, "Save File", "", "", "All files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (saveFileDialog.ShowModal() == wxID_OK) {
            wxString path = saveFileDialog.GetPath();
            wxLogMessage("Saved file: %s", path);
        case wxID_SAVE:
            wxLogMessage("Save tool clicked!");
            break;
        case wxID_SAVE:
            wxLogMessage("Save tool clicked!");
            break;
        }
    }
};

// 定义应用程序类 MyApp，继承自 wxApp
class MyApp : public wxApp {
public:
wxIMPLEMENT_APP(MyApp);        return true;  // 返回 true 表示初始化成功

wxIMPLEMENT_APP(MyApp);
wxIMPLEMENT_APP(MyApp);