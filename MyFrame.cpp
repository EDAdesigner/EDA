#include<wx/wx.h>
#include <wx/toolbar.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/artprov.h>
#include <vector>
#include <wx/frame.h>       // ������ܴ�����ع���
#include <wx/treectrl.h>    // �������ؼ�����ع���
#include <fstream>
#include <nlohmann/json.hpp>
#include "Common.cpp"
#include "DrawPanel.cpp"

using namespace nlohmann; // ʹ�� nlohmann �����ռ�

// ������࣬�̳���wxFrame
class MyFrame : public wxFrame {
    friend class DrawPanel;
public:
    // ���캯��
    MyFrame()
        : wxFrame(nullptr, wxID_ANY, "EDA Example") {

        // ��������壬��ΪӦ�ó���Ļ�������
        wxPanel* panel = new wxPanel(this);
        panel->SetBackgroundColour(*wxLIGHT_GREY); // ���������ı�����ɫΪǳ��ɫ

        // ����ˮƽ���ֹ����������ڹ��������ͻ�ͼ���Ĳ���
        wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);

        // ��������壬��Ϊ�������������ؼ�������
        wxPanel* subPanel = new wxPanel(panel, wxID_ANY);
        subPanel->SetBackgroundColour(*wxLIGHT_GREY); // ���������ı�����ɫΪǳ��ɫ
        hbox->Add(subPanel, 2, wxEXPAND | wxALL, 10); // ���������ӵ������У�����Ϊ2��������չ������ӱ߾�

        // ������ͼ��壬������ʾ�ͻ��Ƶ���Ԫ��
        drawPanel = new DrawPanel(panel);
        hbox->Add(drawPanel, 8, wxEXPAND | wxALL, 10); // ����ͼ�����ӵ������У�����Ϊ1��������չ������ӱ߾�

        // ���������Ĳ��ֹ�����Ϊhbox
        panel->SetSizer(hbox);

        // ����״̬������ʾӦ�ó���״̬
        CreateStatusBar(1);
        SetStatusText("This is a model"); // ����״̬�����ı�

        // �����˵����������ļ��Ͱ����˵�
        wxMenuBar* menuBar = new wxMenuBar;  // �����˵���
        wxMenu* fileMenu = new wxMenu;  // �����ļ��˵�
        // ���ļ��˵������
        fileMenu->Append(wxID_NEW, "&New\tCtrl-N", "Create a new file");
        fileMenu->Append(wxID_OPEN, "&Open\tCtrl-O", "Open a file");
        fileMenu->Append(wxID_SAVE, "&Save\tCtrl-S", "Save the file");
        fileMenu->AppendSeparator();  // ��ӷָ���
        fileMenu->Append(wxID_EXIT, "&Exit\tCtrl-Q", "Exit the application");
        menuBar->Append(fileMenu, "&File");  // ���ļ��˵���ӵ��˵���

        // �����༭�˵�
        wxMenu* editMenu = new wxMenu;
        //��༭�˵��������
        editMenu->Append(ID_CUT, "Cut\tCtrl+X"); // ʹ���Զ��� wxID
        editMenu->AppendSeparator();//��ӷָ���
        editMenu->Append(ID_COPY, "Copy\tCtrl+C");
        editMenu->Append(ID_PASTE, "Paste\tCtrl+V");
        editMenu->AppendSeparator();//��ӷָ���
        editMenu->Append(ID_SELECT_ALL, "Select All\tCtrl+A");
        menuBar->Append(editMenu, "Edit"); // ���༭�˵���ӵ��˵���

        // ������Ŀ�˵�
        wxMenu* projectMenu = new wxMenu;
        projectMenu->Append(wxID_ANY, "Add Circuit...");
        projectMenu->Append(wxID_ANY, "Load Library");
        projectMenu->Append(wxID_ANY, "Unload Libraries...");
        projectMenu->AppendSeparator();//��ӷָ���
        projectMenu->Append(wxID_ANY, "Move Circuit Up");
        projectMenu->Append(wxID_ANY, "Move Circuit Down");
        projectMenu->Append(wxID_ANY, "Set as Main Circuit");
        menuBar->Append(projectMenu, "Project"); // ����Ŀ�˵���ӵ��˵���

        // ����ģ��˵�
        wxMenu* simulateMenu = new wxMenu;
        simulateMenu->Append(wxID_ANY, "Simulation Enabled");
        simulateMenu->Append(wxID_ANY, "Reset Simulation");
        simulateMenu->Append(wxID_ANY, "Step Simulation");
        simulateMenu->AppendSeparator();//��ӷָ���
        simulateMenu->Append(wxID_ANY, "Go Out To State");
        simulateMenu->Append(wxID_ANY, "Go In To State");
        menuBar->Append(simulateMenu, "Simulation"); // ��ģ��˵���ӵ��˵���

        // ���һ����Ϊwindow�Ĳ˵�
        wxMenu* windowMenu = new wxMenu;
        windowMenu->Append(wxID_MAXIMIZE, "&Maximize\tCtrl+M");
        windowMenu->Append(wxID_MINIMIZE, "&Minimize\tCtrl+N");
        windowMenu->Append(wxID_CLOSE, "&Close\tCtrl+W");
        menuBar->Append(windowMenu, "&Window");  // ��window�˵���ӵ��˵���

        // ���������˵�
        wxMenu* helpMenu = new wxMenu;
        helpMenu->Append(wxID_ABOUT, "&About\tF1", "Show about dialog");
        helpMenu->Append(ID_SHOW_TEXT_BOX, "Show Text Box", "Show a new text box");
        menuBar->Append(helpMenu, "&Help");  // �������˵���ӵ��˵���

        SetMenuBar(menuBar); // ����Ӧ�ó���Ĳ˵���
        SetSize(800, 600); // ���ô��ڵĳ�ʼ��С
        Show(true); // ��ʾ����

        // ���������������ڿ��ٷ��ʹ���
        wxToolBar* toolbar = CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT); // ����ˮƽ������
        toolbar->AddTool(wxID_NEW, "New", wxArtProvider::GetBitmap(wxART_NEW)); // ����½�����ͼ��
        toolbar->AddTool(wxID_OPEN, "Open", wxArtProvider::GetBitmap(wxART_FILE_OPEN)); // ��Ӵ򿪹���ͼ��
        toolbar->AddTool(wxID_SAVE, "Save", wxArtProvider::GetBitmap(wxART_FILE_SAVE)); // ��ӱ��湤��ͼ��
        toolbar->Realize(); // ��ɹ������Ĵ���

        // �������ؼ������ڹ���ͬ�ĵ���Ԫ��
        treeCtrl = new wxTreeCtrl(subPanel, wxID_ANY, wxDefaultPosition, wxSize(400, 800), wxTR_DEFAULT_STYLE);

        // �������ڵ�
        wxTreeItemId rootId = treeCtrl->AddRoot("Electronic Components");

        // ��ӵ���Ԫ���ڵ�
        treeCtrl->AppendItem(rootId, "AND Gate");
        treeCtrl->AppendItem(rootId, "OR Gate");
        treeCtrl->AppendItem(rootId, "NOT Gate");
        treeCtrl->AppendItem(rootId, "NAND Gate");
        treeCtrl->AppendItem(rootId, "NOR Gate");
        treeCtrl->AppendItem(rootId, "XOR Gate");
        treeCtrl->AppendItem(rootId, "XNOR Gate");
        treeCtrl->AppendItem(rootId, "Delete");

        // չ�����ڵ�
        treeCtrl->Expand(rootId);

        // �������ؼ��Ĳ���
        wxBoxSizer* treeSizer = new wxBoxSizer(wxVERTICAL);
        treeSizer->Add(treeCtrl, 30, wxEXPAND | wxALL, 10);
        subPanel->SetSizer(treeSizer);

        // ��������С�仯�¼���ȷ�����ؼ��ڴ�С�仯ʱҲ�������
        subPanel->Bind(wxEVT_SIZE, [this](wxSizeEvent& event) {
            wxSize size = event.GetSize();
            // �����ؼ��Ŀ������Ϊ������ȵ�25%
            treeCtrl->SetSize(size.x * 0.25, size.y);
            event.Skip(); // ���������¼�
            });

        // �󶨲˵��¼�����Ӧ�û��Ĳ˵�����
        Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT); // ���˳��¼�
        Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT); // �󶨹����¼�
        Bind(wxEVT_MENU, &MyFrame::OnNew, this, wxID_NEW); // ���½��¼�
        Bind(wxEVT_MENU, &MyFrame::OnOpen, this, wxID_OPEN); // �󶨴��¼�
        Bind(wxEVT_MENU, &MyFrame::OnSave, this, wxID_SAVE); // �󶨱����¼�
        Bind(wxEVT_MENU, &MyFrame::OnMaximize, this, wxID_MAXIMIZE); // ������¼�
        Bind(wxEVT_MENU, &MyFrame::OnMinimize, this, wxID_MINIMIZE); // ����С���¼�
        Bind(wxEVT_MENU, &MyFrame::OnCloseWindow, this, wxID_CLOSE); // �󶨹ر��¼�
        Bind(wxEVT_MENU, &MyFrame::OnCloseWindow, this, wxID_CLOSE); // �󶨹ر��¼�
        Bind(wxEVT_MENU, &MyFrame::OnSelectAll, this, ID_SELECT_ALL); // ��ѡ�������¼�
        Bind(wxEVT_MENU, &MyFrame::OnCopy, this, ID_COPY); // �󶨸����¼�
        Bind(wxEVT_MENU, &MyFrame::OnPaste, this, ID_PASTE); // ��ճ���¼�
        Bind(wxEVT_MENU, &MyFrame::OnCut, this, ID_CUT); // �󶨼����¼�
        Bind(wxEVT_MENU, &MyFrame::OnShowTextBox, this, ID_SHOW_TEXT_BOX);//��helpָ���ĵ�
        // �����ؼ�ѡ���¼�
        treeCtrl->Bind(wxEVT_TREE_SEL_CHANGED, &MyFrame::ToolSelected, this);
    }

    //private:
    wxTreeCtrl* treeCtrl;   // ���ؼ�ָ�룬������ʾ�Ͳ������νṹ�Ŀؼ�
    wxTextCtrl* textBox;    // �ı���ָ�룬������ʾ�ͱ༭�ı����ݵĿؼ�

    // ������ͼ���ָ�룬���ڲ������Ƶ����
    DrawPanel* drawPanel;
    wxString path;//�ļ�·��

    // �����˳��¼�
    void OnExit(wxCommandEvent& event) {
        Close(true); // �ر�Ӧ�ó��򴰿�
    }

    // ������ڶԻ����¼�
    void OnAbout(wxCommandEvent& event) {
        // ��ʾ������Ϣ�ĶԻ���
        wxMessageBox("This is a wxWidgets EDA application.", "About My Application", wxOK | wxICON_INFORMATION);
    }

    // �����½������¼�
    void OnNew(wxCommandEvent& event) {
        // ����һ���µ� MyFrame ����
        MyFrame* newFrame = new MyFrame();
        newFrame->Show(true); // ��ʾ�´���
    }

    // ������ļ��¼�
    void OnOpen(wxCommandEvent& event) {
        // �����ļ��Ի��������û�ѡ��Ҫ�򿪵��ļ�
        wxFileDialog openFileDialog(this, "Open File", "", "", "JSON files (*.json)|*.json", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (openFileDialog.ShowModal() == wxID_OK) { // ��ʾ�Ի��򲢼���û��Ƿ�ѡ�����ļ�
            wxString path = openFileDialog.GetPath(); // ��ȡѡ���ļ���·��
            //wxLogMessage("Opened file: %s", path); // ����־�м�¼���ļ���·��

            // ��ȡ�ļ�����
            std::ifstream file(path.ToStdString());
            if (file.is_open()) {
                json all_component;
                file >> all_component; // ����JSON�ļ�����

                // ��յ�ǰ���
                drawPanel->components.clear();

                // ��JSON����ת��Ϊ���
                for (const auto& component_json : all_component) {
                    DrawPanel::Tool type = static_cast<DrawPanel::Tool>(component_json["type"].get<int>());
                    int x = component_json["x"].get<int>();
                    int y = component_json["y"].get<int>();
                    drawPanel->components.emplace_back(type, wxPoint(x, y));
                }

                // ���»�ͼ���
                drawPanel->Refresh();
            }
            else {
                wxLogError("Cannot open file '%s'.", path);
            }
        }
    }

    //�ṩ�ļ�·���¼�
    void OnProvidePath(wxCommandEvent& event) {
        wxFileDialog openFileDialog(this, "Open File", "", "", "JSON files (*.json)|*.json", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (openFileDialog.ShowModal() == wxID_OK) {
            path = openFileDialog.GetPath();
            //wxLogMessage("OK");
        }
    }

    // �������ļ��¼�
    void OnSave(wxCommandEvent& event) {
        // �����ļ��жԻ��������û�ѡ�񱣴���ļ���λ��
        wxDirDialog saveDirDialog(this, "Select Directory", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        if (saveDirDialog.ShowModal() == wxID_OK) { // ��ʾ�Ի��򲢼���û��Ƿ�ѡ�����ļ���
            wxString dirPath = saveDirDialog.GetPath(); // ��ȡ�û�ѡ����ļ���·��

            // ��������Ի��������û������ļ���
            wxTextEntryDialog fileNameDialog(this, "Enter file name", "File Name", "new_file");
            if (fileNameDialog.ShowModal() == wxID_OK) { // ��ʾ�Ի��򲢼���û��Ƿ��������ļ���
                wxString fileName = fileNameDialog.GetValue() + ".json"; // ��ȡ�û�������ļ���

                // �������ļ�������·��
                wxString filePath = dirPath + "/" + fileName;

                // ����JSON����
                json all_component;
                for (const auto& component : drawPanel->components) {
                    json component_json;
                    component_json["type"] = static_cast<int>(component.first);
                    component_json["x"] = component.second.x;
                    component_json["y"] = component.second.y;
                    all_component.push_back(component_json);
                }

                // ��JSON����д���ļ�
                std::ofstream file(filePath.ToStdString());
                if (file.is_open()) {
                    file << all_component.dump(4);
                    file.close();
                }
                else {
                    wxLogError("Cannot save file '%s'.", filePath);
                }
            }
        }
    }

    // ��󻯴���
    void OnMaximize(wxCommandEvent& event) {
        Maximize(true);
    }

    // ��С������
    void OnMinimize(wxCommandEvent& event) {
        Iconize(true);
    }

    // �رմ���
    void OnCloseWindow(wxCommandEvent& event) {
        Close(true);
    }

    // ѡ�����е��¼�������
    void OnSelectAll(wxCommandEvent& event) {
        drawPanel->SelectAll(); // ���� DrawPanel �е� SelectAll ����
    }

    //����ѡ�е��¼�������
    void OnCopy(wxCommandEvent& event) {
        drawPanel->CopySelected(); // ���� DrawPanel �е� CopySelected ����
    }

    //ճ�����Ƶ��¼�������
    void OnPaste(wxCommandEvent& event) {
        drawPanel->PasteCopied(); // ���� DrawPanel �е� PasteCopied ����
    }

    // ���е��¼�������
    void OnCut(wxCommandEvent& event) {
        drawPanel->CutSelected(); // ���� DrawPanel �е� CutSelected ����
    }


    void OnShowTextBox(wxCommandEvent& event) {
        // ����һ���µ� wxFrame ʵ��
        wxFrame* newFrame = new wxFrame(this, wxID_ANY, "Help Window", wxDefaultPosition, wxSize(1000, 600));

        // ���� wxTreeCtrl �� wxTextCtrl �ؼ�
        treeCtrl = new wxTreeCtrl(newFrame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE);
        textBox = new wxTextCtrl(newFrame, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
        textBox->AppendText("Select a guide from the tree on the left.");

        // �����´��ڵĲ��ֹ�����
        wxBoxSizer* newFrameSizer = new wxBoxSizer(wxHORIZONTAL);
        newFrameSizer->Add(treeCtrl, 1, wxEXPAND | wxALL, 10);  // ������ؼ�
        newFrameSizer->Add(textBox, 3, wxEXPAND | wxALL, 10);  // ����ı���Ȩ��Ϊ3
        newFrame->SetSizer(newFrameSizer);

        // �������ڵ�
        wxTreeItemId rootId = treeCtrl->AddRoot("Guides");

        // ����ӽڵ�
        treeCtrl->AppendItem(rootId, "Guide 1");
        treeCtrl->AppendItem(rootId, "Guide 2");
        treeCtrl->AppendItem(rootId, "Guide 3");
        treeCtrl->AppendItem(rootId, "Guide 4");
        treeCtrl->AppendItem(rootId, "Guide 5");

        // չ�����ڵ�
        treeCtrl->Expand(rootId);

        // �����ؼ����¼��������
        treeCtrl->Bind(wxEVT_TREE_SEL_CHANGED, &MyFrame::OnTreeItemSelected, this);

        // ���´��ھ�����ʾ
        newFrame->Center();

        // ��ʾ�´���
        newFrame->Show();
    }


    //�������ؼ�ѡ���¼��ĺ���
    void OnTreeItemSelected(wxTreeEvent& event) {
        wxTreeItemId itemId = event.GetItem();
        wxString nodeName = treeCtrl->GetItemText(itemId);
       

        // ������ѡ�ڵ����Ƹ����ı�������
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


    // �������ؼ�ѡ���¼��ĺ���
    void ToolSelected(wxTreeEvent& event) {
        wxTreeItemId itemId = event.GetItem();
        wxString nodeName = treeCtrl->GetItemText(itemId);

        // ������ѡ�ڵ�����ѡ����Ӧ�Ĺ���
        if (nodeName == "AND Gate") {
            drawPanel->SetCurrentTool(DrawPanel::Tool::AND_GATE);
        }
        else if (nodeName == "OR Gate") {
            drawPanel->SetCurrentTool(DrawPanel::Tool::OR_GATE);
        }
        else if (nodeName == "NOT Gate") {
            drawPanel->SetCurrentTool(DrawPanel::Tool::NOT_GATE);
        }
        else if (nodeName == "NAND Gate") {
            drawPanel->SetCurrentTool(DrawPanel::Tool::NAND_GATE);
        }
        else if (nodeName == "NOR Gate") {
            drawPanel->SetCurrentTool(DrawPanel::Tool::NOR_GATE);
        }
        else if (nodeName == "XOR Gate") {
            drawPanel->SetCurrentTool(DrawPanel::Tool::XOR_GATE);
        }
        else if (nodeName == "XNOR Gate") {
            drawPanel->SetCurrentTool(DrawPanel::Tool::XNOR_GATE);
        }
    }
};
