#include<wx/wx.h>
#include <wx/toolbar.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/artprov.h>
#include <vector>
#include <wx/frame.h>       // 包含框架窗口相关功能
#include <wx/treectrl.h>    // 包含树控件的相关功能
#include <fstream>
#include <nlohmann/json.hpp>
#include "Common.cpp"
#include "DrawPanel.cpp"

using namespace nlohmann; // 使用 nlohmann 命名空间

// 主框架类，继承自wxFrame
class MyFrame : public wxFrame {
    friend class DrawPanel;
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
        hbox->Add(drawPanel, 8, wxEXPAND | wxALL, 10); // 将绘图面板添加到布局中，比例为1，允许扩展，并添加边距

        // 设置主面板的布局管理器为hbox
        panel->SetSizer(hbox);

        // 创建状态栏，显示应用程序状态
        CreateStatusBar(1);
        SetStatusText("This is a model"); // 设置状态栏的文本

        // 创建菜单栏，包含文件和帮助菜单
        wxMenuBar* menuBar = new wxMenuBar;  // 创建菜单栏
        wxMenu* fileMenu = new wxMenu;  // 创建文件菜单
        // 向文件菜单添加项
        fileMenu->Append(wxID_NEW, "&New\tCtrl-N", "Create a new file");
        fileMenu->Append(wxID_OPEN, "&Open\tCtrl-O", "Open a file");
        fileMenu->Append(wxID_SAVE, "&Save\tCtrl-S", "Save the file");
        fileMenu->AppendSeparator();  // 添加分隔符
        fileMenu->Append(wxID_EXIT, "&Exit\tCtrl-Q", "Exit the application");
        menuBar->Append(fileMenu, "&File");  // 将文件菜单添加到菜单栏

        // 创建编辑菜单
        wxMenu* editMenu = new wxMenu;
        //向编辑菜单中添加项
        editMenu->Append(ID_CUT, "Cut\tCtrl+X"); // 使用自定义 wxID
        editMenu->AppendSeparator();//添加分隔符
        editMenu->Append(ID_COPY, "Copy\tCtrl+C");
        editMenu->Append(ID_PASTE, "Paste\tCtrl+V");
        editMenu->AppendSeparator();//添加分隔符
        editMenu->Append(ID_SELECT_ALL, "Select All\tCtrl+A");
        menuBar->Append(editMenu, "Edit"); // 将编辑菜单添加到菜单栏

        // 创建项目菜单
        wxMenu* projectMenu = new wxMenu;
        projectMenu->Append(wxID_ANY, "Add Circuit...");
        projectMenu->Append(wxID_ANY, "Load Library");
        projectMenu->Append(wxID_ANY, "Unload Libraries...");
        projectMenu->AppendSeparator();//添加分隔符
        projectMenu->Append(wxID_ANY, "Move Circuit Up");
        projectMenu->Append(wxID_ANY, "Move Circuit Down");
        projectMenu->Append(wxID_ANY, "Set as Main Circuit");
        menuBar->Append(projectMenu, "Project"); // 将项目菜单添加到菜单栏

        // 创建模拟菜单
        wxMenu* simulateMenu = new wxMenu;
        simulateMenu->Append(wxID_ANY, "Simulation Enabled");
        simulateMenu->Append(wxID_ANY, "Reset Simulation");
        simulateMenu->Append(wxID_ANY, "Step Simulation");
        simulateMenu->AppendSeparator();//添加分隔符
        simulateMenu->Append(wxID_ANY, "Go Out To State");
        simulateMenu->Append(wxID_ANY, "Go In To State");
        menuBar->Append(simulateMenu, "Simulation"); // 将模拟菜单添加到菜单栏

        // 添加一个名为window的菜单
        wxMenu* windowMenu = new wxMenu;
        windowMenu->Append(wxID_MAXIMIZE, "&Maximize\tCtrl+M");
        windowMenu->Append(wxID_MINIMIZE, "&Minimize\tCtrl+N");
        windowMenu->Append(wxID_CLOSE, "&Close\tCtrl+W");
        menuBar->Append(windowMenu, "&Window");  // 将window菜单添加到菜单栏

        // 创建帮助菜单
        wxMenu* helpMenu = new wxMenu;
        helpMenu->Append(wxID_ABOUT, "&About\tF1", "Show about dialog");
        helpMenu->Append(ID_SHOW_TEXT_BOX, "Show Text Box", "Show a new text box");
        menuBar->Append(helpMenu, "&Help");  // 将帮助菜单添加到菜单栏

        SetMenuBar(menuBar); // 设置应用程序的菜单栏
        SetSize(800, 600); // 设置窗口的初始大小
        Show(true); // 显示窗口

        // 创建AND门图标
        wxInitAllImageHandlers();
        wxBitmap bitmapForAndgate;
        if (!bitmapForAndgate.LoadFile("resource/AND gate.png", wxBITMAP_TYPE_PNG)) {
            wxLogError("Failed to load AND gate icon.");
        }
        else {
            wxImage imageForAnd = bitmapForAndgate.ConvertToImage();
            imageForAnd = imageForAnd.Scale(30, 30, wxIMAGE_QUALITY_HIGH);
            bitmapForAndgate = wxBitmap(imageForAnd);
        }
        // 加载 NOT 门 图标
        wxBitmap bitmapForNOTgate;
        if (!bitmapForNOTgate.LoadFile("resource/NOT gate.png", wxBITMAP_TYPE_PNG)) {
            wxLogError("Failed to load NOT gate icon.");
        }
        else {
            wxImage imageForNOTgate = bitmapForNOTgate.ConvertToImage();
            imageForNOTgate = imageForNOTgate.Scale(30, 30, wxIMAGE_QUALITY_HIGH);
            bitmapForNOTgate = wxBitmap(imageForNOTgate);
        }

        // 加载线条图标
        wxBitmap bitmapForLINE;
        if (!bitmapForLINE.LoadFile("resource/LINE.png", wxBITMAP_TYPE_PNG)) {
            wxLogError("Failed to load NOT gate icon.");
        }
        else {
            wxImage imageForLINE = bitmapForLINE.ConvertToImage();
            imageForLINE = imageForLINE.Scale(30, 30, wxIMAGE_QUALITY_HIGH);
            bitmapForLINE = wxBitmap(imageForLINE);
        }
        // 加载箭头图标
        wxBitmap bitmapForARROW;
        if (!bitmapForARROW.LoadFile("resource/ARROW.png", wxBITMAP_TYPE_PNG)) {
            wxLogError("Failed to load NOT gate icon.");
        }
        else {
            wxImage imageForARROW = bitmapForARROW.ConvertToImage();
            imageForARROW = imageForARROW.Scale(30, 30, wxIMAGE_QUALITY_HIGH);
            bitmapForARROW = wxBitmap(imageForARROW);
        }

        // 加载 OR门图标
        wxBitmap bitmapForORgate;
        if (!bitmapForORgate.LoadFile("resource/OR gate.png", wxBITMAP_TYPE_PNG)) {
            wxLogError("Failed to load OR gate icon.");
        }
        else {
            wxImage imageForORgate = bitmapForORgate.ConvertToImage();
            imageForORgate = imageForORgate.Scale(30, 30, wxIMAGE_QUALITY_HIGH);
            bitmapForORgate = wxBitmap(imageForORgate);
        }

        // 创建工具栏，用于快速访问功能
        wxToolBar* toolbar = CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT); // 创建水平工具栏
        toolbar->AddTool(wxID_NEW, "New", wxArtProvider::GetBitmap(wxART_NEW)); // 添加新建工具图标
        toolbar->AddTool(wxID_OPEN, "Open", wxArtProvider::GetBitmap(wxART_FILE_OPEN)); // 添加打开工具图标
        toolbar->AddTool(wxID_SAVE, "Save", wxArtProvider::GetBitmap(wxART_FILE_SAVE)); // 添加保存工具图标
        if (bitmapForAndgate.IsOk()) {
            toolbar->AddTool(wxID_AND_GATE, "AND Gate", bitmapForAndgate);
        }
        if (bitmapForNOTgate.IsOk()) {
            toolbar->AddTool(wxID_NOT_GATE, "NOT Gate", bitmapForNOTgate);
        }
        if (bitmapForORgate.IsOk()) {
            toolbar->AddTool(wxID_OR_GATE, "OR Gate", bitmapForORgate);
        }
        if (bitmapForLINE.IsOk()) {
            toolbar->AddTool(wxID_LINE, "CONNECT", bitmapForLINE);
        }
        if (bitmapForARROW.IsOk()) {
            toolbar->AddTool(wxID_ARROW, "ARROW", bitmapForARROW);
        }



        toolbar->Realize(); // 完成工具栏的创建

        // 创建树控件，用于管理不同的电子元件
        treeCtrl = new wxTreeCtrl(subPanel, wxID_ANY, wxDefaultPosition, wxSize(400, 800), wxTR_DEFAULT_STYLE);

        // 创建根节点
        wxTreeItemId rootId = treeCtrl->AddRoot("Electronic Components");

        // 添加电子元件节点
        treeCtrl->AppendItem(rootId, "AND Gate");
        treeCtrl->AppendItem(rootId, "OR Gate");
        treeCtrl->AppendItem(rootId, "NOT Gate");
        treeCtrl->AppendItem(rootId, "NAND Gate");
        treeCtrl->AppendItem(rootId, "NOR Gate");
        treeCtrl->AppendItem(rootId, "XOR Gate");
        treeCtrl->AppendItem(rootId, "XNOR Gate");
        treeCtrl->AppendItem(rootId, "Delete");

        // 展开根节点
        treeCtrl->Expand(rootId);

        // 设置树控件的布局
        wxBoxSizer* treeSizer = new wxBoxSizer(wxVERTICAL);
        treeSizer->Add(treeCtrl, 30, wxEXPAND | wxALL, 10);
        subPanel->SetSizer(treeSizer);

        // 绑定子面板大小变化事件，确保树控件在大小变化时也跟随调整
        subPanel->Bind(wxEVT_SIZE, [this](wxSizeEvent& event) {
            wxSize size = event.GetSize();
            // 将树控件的宽度设置为子面板宽度的25%
            treeCtrl->SetSize(size.x * 0.25, size.y);
            event.Skip(); // 继续处理事件
            });

        // 绑定菜单事件，响应用户的菜单操作
        Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT); // 绑定退出事件
        Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT); // 绑定关于事件
        Bind(wxEVT_MENU, &MyFrame::OnNew, this, wxID_NEW); // 绑定新建事件
        Bind(wxEVT_MENU, &MyFrame::OnOpen, this, wxID_OPEN); // 绑定打开事件
        Bind(wxEVT_MENU, &MyFrame::OnSave, this, wxID_SAVE); // 绑定保存事件
        Bind(wxEVT_MENU, &MyFrame::OnMaximize, this, wxID_MAXIMIZE); // 绑定最大化事件
        Bind(wxEVT_MENU, &MyFrame::OnMinimize, this, wxID_MINIMIZE); // 绑定最小化事件
        Bind(wxEVT_MENU, &MyFrame::OnCloseWindow, this, wxID_CLOSE); // 绑定关闭事件
        Bind(wxEVT_MENU, &MyFrame::OnCloseWindow, this, wxID_CLOSE); // 绑定关闭事件
        Bind(wxEVT_MENU, &MyFrame::OnSelectAll, this, ID_SELECT_ALL); // 绑定选择所有事件
        Bind(wxEVT_MENU, &MyFrame::OnCopy, this, ID_COPY); // 绑定复制事件
        Bind(wxEVT_MENU, &MyFrame::OnPaste, this, ID_PASTE); // 绑定粘贴事件
        Bind(wxEVT_MENU, &MyFrame::OnCut, this, ID_CUT); // 绑定剪切事件
        Bind(wxEVT_MENU, &MyFrame::OnShowTextBox, this, ID_SHOW_TEXT_BOX);//绑定help指导文档
        Bind(wxEVT_MENU, &MyFrame::OnConnectButtonClick, this, wxID_LINE); // 绑定lianxian事件

        // 绑定树控件选择事件
        treeCtrl->Bind(wxEVT_TREE_SEL_CHANGED, &MyFrame::ToolSelected, this);
    }

    //private:
    wxTreeCtrl* treeCtrl;   // 树控件指针，用于显示和操作树形结构的控件
    wxTextCtrl* textBox;    // 文本框指针，用于显示和编辑文本内容的控件

    // 声明绘图面板指针，用于操作绘制的组件
    DrawPanel* drawPanel;
    wxString path;//文件路径

    // 进入连线模式
    void OnConnectButtonClick(wxCommandEvent& event) {
        drawPanel->connecting=true;  // 设置进入连线模式
    }

    // 处理退出事件
    void OnExit(wxCommandEvent& event) {
        Close(true); // 关闭应用程序窗口
    }

    // 处理关于对话框事件
    void OnAbout(wxCommandEvent& event) {
        // 显示关于信息的对话框
        wxMessageBox("This is a wxWidgets EDA application.", "About My Application", wxOK | wxICON_INFORMATION);
    }

    // 处理新建窗口事件
    void OnNew(wxCommandEvent& event) {
        // 创建一个新的 MyFrame 窗口
        MyFrame* newFrame = new MyFrame();
        newFrame->Show(true); // 显示新窗口
    }

    // 处理打开文件事件
    void OnOpen(wxCommandEvent& event) {
        // 创建文件对话框，允许用户选择要打开的文件
        wxFileDialog openFileDialog(this, "Open File", "", "", "JSON files (*.json)|*.json", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (openFileDialog.ShowModal() == wxID_OK) { // 显示对话框并检查用户是否选择了文件
            wxString path = openFileDialog.GetPath(); // 获取选定文件的路径

            // 打开并读取文件
            std::ifstream file(path.ToStdString());
            if (file.is_open()) {
                json final_json;
                file >> final_json; // 读取JSON对象
                file.close();

                // 清空当前的组件和连接
                drawPanel->components.clear();
                drawPanel->connections.clear();

                // 读取组件数据
                for (const auto& component_json : final_json["components"]) {
                    Component::Tool tool = static_cast<Component::Tool>(component_json["type"].get<int>());
                    wxPoint position(component_json["x"].get<int>(), component_json["y"].get<int>());
                    Component component(tool, position);

                    // 读取引脚数据
                    for (const auto& pin_json : component_json["pins"]) {
                        wxPoint start(pin_json["start"]["x"].get<int>(), pin_json["start"]["y"].get<int>());
                        wxPoint end(pin_json["end"]["x"].get<int>(), pin_json["end"]["y"].get<int>());
                        component.pins.push_back(std::make_pair(start, end));
                    }

                    drawPanel->components.push_back(component);
                }

                // 读取连接数据
                for (const auto& connection_json : final_json["connections"]) {
                    wxPoint start(connection_json["start"]["x"].get<int>(), connection_json["start"]["y"].get<int>());
                    wxPoint end(connection_json["end"]["x"].get<int>(), connection_json["end"]["y"].get<int>());
                    drawPanel->connections.push_back(std::make_pair(start, end));
                }

                // 重新绘制面板
                drawPanel->Refresh();
            }
            else {
                wxLogError("Cannot open file '%s'.", path);
            }
        }
    }

    //提供文件路径事件
    void OnProvidePath(wxCommandEvent& event) {
        wxFileDialog openFileDialog(this, "Open File", "", "", "JSON files (*.json)|*.json", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (openFileDialog.ShowModal() == wxID_OK) {
            path = openFileDialog.GetPath();
            //wxLogMessage("OK");
        }
    }

    // 处理保存文件事件
    void OnSave(wxCommandEvent& event) {
        // 创建文件对话框，允许用户选择保存文件的位置
        wxFileDialog saveFileDialog(this, "Save File", "", "", "JSON files (*.json)|*.json", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (saveFileDialog.ShowModal() == wxID_OK) { // 显示对话框并检查用户是否选择了文件
            wxString path = saveFileDialog.GetPath(); // 获取选定文件的路径

            // 创建JSON对象
            json all_component = json::array();

            // 将组件转换为JSON对象
            for (const auto& component : drawPanel->components) {
                json component_json;
                component_json["type"] = static_cast<int>(component.tool);
                component_json["x"] = component.position.x;
                component_json["y"] = component.position.y;

                // 将引脚数据转换为JSON对象
                json pins_json = json::array();
                for (const auto& pin : component.pins) {
                    json pin_json;
                    pin_json["start"] = { {"x", pin.first.x}, {"y", pin.first.y} };
                    pin_json["end"] = { {"x", pin.second.x}, {"y", pin.second.y} };
                    pins_json.push_back(pin_json);
                }
                component_json["pins"] = pins_json;

                all_component.push_back(component_json);
            }

            // 将连接转换为JSON对象
            json all_connections = json::array();
            for (const auto& connection : drawPanel->connections) {
                json connection_json;
                connection_json["start"] = { {"x", connection.first.x}, {"y", connection.first.y} };
                connection_json["end"] = { {"x", connection.second.x}, {"y", connection.second.y} };
                all_connections.push_back(connection_json);
            }



            // 创建最终的JSON对象
            json final_json;
            final_json["components"] = all_component;
            final_json["connections"] = all_connections;

            // 将JSON对象写入文件
            std::ofstream file(path.ToStdString());
            if (file.is_open()) {
                file << final_json.dump(4); // 以缩进4个空格的格式写入文件
                file.close();
            }
            else {
                wxLogError("Cannot save file '%s'.", path);
            }
        }
    }

    // 最大化窗口
    void OnMaximize(wxCommandEvent& event) {
        Maximize(true);
    }

    // 最小化窗口
    void OnMinimize(wxCommandEvent& event) {
        Iconize(true);
    }

    // 关闭窗口
    void OnCloseWindow(wxCommandEvent& event) {
        Close(true);
    }

    // 选择所有的事件处理函数
    void OnSelectAll(wxCommandEvent& event) {
    //    drawPanel->SelectAll(); // 调用 DrawPanel 中的 SelectAll 方法
    }

    //复制选中的事件处理函数
    void OnCopy(wxCommandEvent& event) {
    //    drawPanel->CopySelected(); // 调用 DrawPanel 中的 CopySelected 方法
    }

    //粘贴复制的事件处理函数
    void OnPaste(wxCommandEvent& event) {
    //    drawPanel->PasteCopied(); // 调用 DrawPanel 中的 PasteCopied 方法
    }

    // 剪切的事件处理函数
    void OnCut(wxCommandEvent& event) {
    //    drawPanel->CutSelected(); // 调用 DrawPanel 中的 CutSelected 方法
    }


    void OnShowTextBox(wxCommandEvent& event) {
        // 创建一个新的 wxFrame 实例
        wxFrame* newFrame = new wxFrame(this, wxID_ANY, "Help Window", wxDefaultPosition, wxSize(1000, 600));

        // 创建 wxTreeCtrl 和 wxTextCtrl 控件
        treeCtrl = new wxTreeCtrl(newFrame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE);
        textBox = new wxTextCtrl(newFrame, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
        textBox->AppendText("Select a guide from the tree on the left.");

        // 设置新窗口的布局管理器
        wxBoxSizer* newFrameSizer = new wxBoxSizer(wxHORIZONTAL);
        newFrameSizer->Add(treeCtrl, 1, wxEXPAND | wxALL, 10);  // 添加树控件
        newFrameSizer->Add(textBox, 3, wxEXPAND | wxALL, 10);  // 添加文本框，权重为3
        newFrame->SetSizer(newFrameSizer);

        // 创建根节点
        wxTreeItemId rootId = treeCtrl->AddRoot("Guides");

        // 添加子节点
        treeCtrl->AppendItem(rootId, "Guide 1");
        treeCtrl->AppendItem(rootId, "Guide 2");
        treeCtrl->AppendItem(rootId, "Guide 3");
        treeCtrl->AppendItem(rootId, "Guide 4");
        treeCtrl->AppendItem(rootId, "Guide 5");

        // 展开根节点
        treeCtrl->Expand(rootId);

        // 绑定树控件的事件处理程序
        treeCtrl->Bind(wxEVT_TREE_SEL_CHANGED, &MyFrame::OnTreeItemSelected, this);

        // 将新窗口居中显示
        newFrame->Center();

        // 显示新窗口
        newFrame->Show();
    }


    //处理树控件选择事件的函数
    void OnTreeItemSelected(wxTreeEvent& event) {
        wxTreeItemId itemId = event.GetItem();
        wxString nodeName = treeCtrl->GetItemText(itemId);
       

        // 根据所选节点名称更新文本框内容
        if (nodeName == "Guide 1") {
            textBox->SetValue("Content for Guide 1: Lorem ipsum dolor sit amet...");
        }
        else if (nodeName == "Guide 2") {
            textBox->SetValue("Content for Guide 2: Consectetur adipiscing elit...");
        }
        else if (nodeName == "Guide 3") {
            textBox->SetValue("Content for Guide 3: Sed do eiusmod tempor incididunt...");
        }
        else if (nodeName == "Guide 4") {
            textBox->SetValue("Content for Guide 4: Ut labore et dolore magna aliqua...");
        }
        else if (nodeName == "Guide 5") {
            textBox->SetValue("Content for Guide 5: Duis aute irure dolor in reprehenderit...");
        }
    }


    // 处理树控件选择事件的函数
    void ToolSelected(wxTreeEvent& event) {
        wxTreeItemId itemId = event.GetItem();
        wxString nodeName = treeCtrl->GetItemText(itemId);

        // 根据所选节点名称选择相应的工具
        if (nodeName == "AND Gate") {
            drawPanel->SetCurrentTool(Component::Tool::AND_GATE);
        }
        else if (nodeName == "OR Gate") {
            drawPanel->SetCurrentTool(Component::Tool::OR_GATE);
        }
        else if (nodeName == "NOT Gate") {
            drawPanel->SetCurrentTool(Component::Tool::NOT_GATE);
        }
        else if (nodeName == "NAND Gate") {
            drawPanel->SetCurrentTool(Component::Tool::NAND_GATE);
        }
        else if (nodeName == "NOR Gate") {
            drawPanel->SetCurrentTool(Component::Tool::NOR_GATE);
        }
        else if (nodeName == "XOR Gate") {
            drawPanel->SetCurrentTool(Component::Tool::XOR_GATE);
        }
        else if (nodeName == "XNOR Gate") {
            drawPanel->SetCurrentTool(Component::Tool::XNOR_GATE);
        }
    }
};
