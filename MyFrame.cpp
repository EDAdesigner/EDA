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
#include <map>
#include <tuple>
#include <stack>

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

        // ����AND��ͼ��
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
        // ���� NOT �� ͼ��
        wxBitmap bitmapForNOTgate;
        if (!bitmapForNOTgate.LoadFile("resource/NOT gate.png", wxBITMAP_TYPE_PNG)) {
            wxLogError("Failed to load NOT gate icon.");
        }
        else {
            wxImage imageForNOTgate = bitmapForNOTgate.ConvertToImage();
            imageForNOTgate = imageForNOTgate.Scale(30, 30, wxIMAGE_QUALITY_HIGH);
            bitmapForNOTgate = wxBitmap(imageForNOTgate);
        }

        // ��������ͼ��
        wxBitmap bitmapForLINE;
        if (!bitmapForLINE.LoadFile("resource/LINE.png", wxBITMAP_TYPE_PNG)) {
            wxLogError("Failed to load NOT gate icon.");
        }
        else {
            wxImage imageForLINE = bitmapForLINE.ConvertToImage();
            imageForLINE = imageForLINE.Scale(30, 30, wxIMAGE_QUALITY_HIGH);
            bitmapForLINE = wxBitmap(imageForLINE);
        }
        // ���ؼ�ͷͼ��
        wxBitmap bitmapForARROW;
        if (!bitmapForARROW.LoadFile("resource/ARROW.png", wxBITMAP_TYPE_PNG)) {
            wxLogError("Failed to load NOT gate icon.");
        }
        else {
            wxImage imageForARROW = bitmapForARROW.ConvertToImage();
            imageForARROW = imageForARROW.Scale(30, 30, wxIMAGE_QUALITY_HIGH);
            bitmapForARROW = wxBitmap(imageForARROW);
        }

        // ���� OR��ͼ��
        wxBitmap bitmapForORgate;
        if (!bitmapForORgate.LoadFile("resource/OR gate.png", wxBITMAP_TYPE_PNG)) {
            wxLogError("Failed to load OR gate icon.");
        }
        else {
            wxImage imageForORgate = bitmapForORgate.ConvertToImage();
            imageForORgate = imageForORgate.Scale(30, 30, wxIMAGE_QUALITY_HIGH);
            bitmapForORgate = wxBitmap(imageForORgate);
        }

        // ���������������ڿ��ٷ��ʹ���
        wxToolBar* toolbar = CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT); // ����ˮƽ������
        toolbar->AddTool(wxID_NEW, "New", wxArtProvider::GetBitmap(wxART_NEW)); // ����½�����ͼ��
        toolbar->AddTool(wxID_OPEN, "Open", wxArtProvider::GetBitmap(wxART_FILE_OPEN)); // ��Ӵ򿪹���ͼ��
        toolbar->AddTool(wxID_SAVE, "Save", wxArtProvider::GetBitmap(wxART_FILE_SAVE)); // ��ӱ��湤��ͼ��
        toolbar->AddTool(wxID_NEW_BUTTON, "New Button", wxArtProvider::GetBitmap(wxART_HELP)); // ʹ�ð���ͼ����Ϊʾ��

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
        Bind(wxEVT_MENU, &MyFrame::OnConnectButtonClick, this, wxID_LINE); // ��lianxian�¼�
        Bind(wxEVT_MENU, &MyFrame::Light, this, wxID_NEW_BUTTON);

        // �����ؼ�ѡ���¼�
        treeCtrl->Bind(wxEVT_TREE_SEL_CHANGED, &MyFrame::ToolSelected, this);
    }

    //private:
    wxTreeCtrl* treeCtrl;   // ���ؼ�ָ�룬������ʾ�Ͳ������νṹ�Ŀؼ�
    wxTextCtrl* textBox;    // �ı���ָ�룬������ʾ�ͱ༭�ı����ݵĿؼ�

    // ������ͼ���ָ�룬���ڲ������Ƶ����
    DrawPanel* drawPanel;
    wxString path;//�ļ�·��

    // ��������ģʽ
    void OnConnectButtonClick(wxCommandEvent& event) {
        drawPanel->connecting=true;  // ���ý�������ģʽ
    }

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

            // �򿪲���ȡ�ļ�
            std::ifstream file(path.ToStdString());
            if (file.is_open()) {
                json final_json;
                file >> final_json; // ��ȡJSON����
                file.close();

                // ��յ�ǰ�����������
                drawPanel->components.clear();
                drawPanel->connections.clear();

                // ��ȡ�������
                for (const auto& component_json : final_json["components"]) {
                    Component::Tool tool = static_cast<Component::Tool>(component_json["type"].get<int>());
                    wxPoint position(component_json["x"].get<int>(), component_json["y"].get<int>());
                    Component component(tool, position);

                    // ��ȡ��������
                    for (const auto& pin_json : component_json["pins"]) {
                        wxPoint start(pin_json["start"]["x"].get<int>(), pin_json["start"]["y"].get<int>());
                        wxPoint end(pin_json["end"]["x"].get<int>(), pin_json["end"]["y"].get<int>());
                        component.pins.push_back(std::make_pair(start, end));
                    }

                    drawPanel->components.push_back(component);
                }

                // ��ȡ��������
                for (const auto& connection_json : final_json["connections"]) {
                    wxPoint start(connection_json["start"]["x"].get<int>(), connection_json["start"]["y"].get<int>());
                    wxPoint end(connection_json["end"]["x"].get<int>(), connection_json["end"]["y"].get<int>());
                    drawPanel->connections.push_back(std::make_pair(start, end));
                }

                // ���»������
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
        // �����ļ��Ի��������û�ѡ�񱣴��ļ���λ��
        wxFileDialog saveFileDialog(this, "Save File", "", "", "JSON files (*.json)|*.json", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (saveFileDialog.ShowModal() == wxID_OK) { // ��ʾ�Ի��򲢼���û��Ƿ�ѡ�����ļ�
            wxString path = saveFileDialog.GetPath(); // ��ȡѡ���ļ���·��

            // ����JSON����
            json all_component = json::array();

            // �����ת��ΪJSON����
            for (const auto& component : drawPanel->components) {
                json component_json;
                component_json["type"] = static_cast<int>(component.tool);
                component_json["x"] = component.position.x;
                component_json["y"] = component.position.y;

                // ����������ת��ΪJSON����
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

            // ������ת��ΪJSON����
            json all_connections = json::array();
            for (const auto& connection : drawPanel->connections) {
                json connection_json;
                connection_json["start"] = { {"x", connection.first.x}, {"y", connection.first.y} };
                connection_json["end"] = { {"x", connection.second.x}, {"y", connection.second.y} };
                all_connections.push_back(connection_json);
            }



            // �������յ�JSON����
            json final_json;
            final_json["components"] = all_component;
            final_json["connections"] = all_connections;

            // ��JSON����д���ļ�
            std::ofstream file(path.ToStdString());
            if (file.is_open()) {
                file << final_json.dump(4); // ������4���ո�ĸ�ʽд���ļ�
                file.close();
            }
            else {
                wxLogError("Cannot save file '%s'.", path);
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
    //    drawPanel->SelectAll(); // ���� DrawPanel �е� SelectAll ����
    }

    //����ѡ�е��¼�������
    void OnCopy(wxCommandEvent& event) {
    //    drawPanel->CopySelected(); // ���� DrawPanel �е� CopySelected ����
    }

    //ճ�����Ƶ��¼�������
    void OnPaste(wxCommandEvent& event) {
    //    drawPanel->PasteCopied(); // ���� DrawPanel �е� PasteCopied ����
    }

    // ���е��¼�������
    void OnCut(wxCommandEvent& event) {
    //    drawPanel->CutSelected(); // ���� DrawPanel �е� CutSelected ����
    }

    void Light(wxCommandEvent& event) {

        //���components
        std::vector<Component> components = drawPanel->components;
        //��ȡconnections
        std::vector<std::pair<wxPoint, wxPoint>> connections = drawPanel->connections;

	    std::stack<Component> stack1;
        std::stack<Component> stack2;

        //�ӵ��ݳ�����������������Ԫ����Ӧ�ķ��ű��ʽѹ��ջ��,ֱ���޷��ҵ����Ӷ���������Ӷ���Ϊ���
        for (auto& component : components) {
            if (component.tool == Component::Tool::BULB) {
                stack1.push(component);
                break;
            }
        }
		if (stack1.empty()) {
			wxLogMessage("No bulb found");
			return;
		}

        Component noneComponent(Component::Tool::NONE, wxPoint(-1, -1));
        while (!stack1.empty()) {
			Component current = stack1.top();
			stack1.pop();

			stack2.push(current);

			/*��currentԪ�����ӵĶ������ʶ��
            ������Ƿ��ţ������������������ţ���Ҫ�Ƚ����ӵ�Ԫ��ѹ��ջ�У����ҵ������������Ŷ�Ӧ��Ԫ��
            ����Ƿ��ţ�������һ���������ţ���Ҫ�Ƚ����ӵ�Ԫ��ѹ��ջ�У����ҵ�һ����Ӧ��Ԫ������
            ����ǵ�Դ����ѹ���ԴԪ�������û�����Ӷ��󣬾ʹ���һ���յ�Component��tool��NONE*/

            // �� current Ԫ�����ӵĶ������ʶ��
            if (current.tool != Component::Tool::NOT_GATE || current.tool != Component::Tool::BULB || current.tool != Component::Tool::BATTERY) {
                // ���Ƿ��ź͵��ݣ���������������
                for (auto& connection : connections) {
                    if (connection.second == current.pins[0].second) {
                        wxPoint nextPosition = connection.first;
                        for (auto& component : components) {
                            if (current.tool != Component::Tool::NOT_GATE) {
                                if (component.pins[2].second == nextPosition) {
                                    stack1.push(component);
                                }
                                else {
                                    stack1.push(noneComponent);
                                }
							}
                            else {
                                if (component.pins[1].second == nextPosition) {
                                    stack1.push(component);
                                }
                                else {
                                    stack1.push(noneComponent);
                                }
                            }
                        }
                    }
					if (connection.second == current.pins[1].second) {
						wxPoint nextPosition = connection.first;
                        for (auto& component : components) {
                            if (current.tool != Component::Tool::NOT_GATE) {
                                if (component.pins[2].second == nextPosition) {
                                    stack1.push(component);
                                }
                                else {
                                    stack1.push(noneComponent);
                                }
                            }
                            else {
                                if (component.pins[1].second == nextPosition) {
                                    stack1.push(component);
                                }
                                else {
                                    stack1.push(noneComponent);
                                }
                            }
                        }
					}
                }
            }
            else if (current.tool == Component::Tool::NOT_GATE || current.tool == Component::Tool::BULB){
                // �ǵ��ݻ���ݣ���һ����������
                for (auto& connection : connections) {
                    if (connection.second == current.pins[0].second) {
                        wxPoint nextPosition = connection.first;
                        for (auto& component : components) {
                            if (current.tool != Component::Tool::NOT_GATE) {
                                if (component.pins[2].second == nextPosition) {
                                    stack1.push(component);
                                }
                                else {
                                    stack1.push(noneComponent);
                                }
                            }
                            else {
                                if (component.pins[1].second == nextPosition) {
                                    stack1.push(component);
                                }
								else {
									stack1.push(noneComponent);
								}
                            }
                        }
                    }
                }
            }

            // ����ǵ�Դ��ѹ���ԴԪ��
            if (current.tool == Component::Tool::BATTERY) {
            }
        }

		// ��ջ�е���Ԫ�����������

        std::vector<bool> output = { false };
        int i = 0;
		bool j = false;
		bool k = false;
		bool m = false;
		bool n = false;
		bool o = false;
		bool p = false;


        while (!stack2.empty()) {
            Component current = stack2.top();
            stack2.pop();

            switch (current.tool) {
                case Component::Tool::NOT_GATE:
                    // ���ŵ����������ķ�
                    output[i] = !output[i];
                    break;
                case Component::Tool::BATTERY:
                    // ��ص�����Ǻ㶨��
                    output[i++] = true;
                    break;
                case Component::Tool::OR_GATE:
					// ���ŵ����������Ļ�
					k = output[i] || output[i-1];
                    output[i - 1] = k;
                    i--;
                    break;
                case Component::Tool::AND_GATE:
					// ����de������������
					j = output[i] && output[i - 1];
					output[i - 1] = j;
					i--;
					break;
			    case Component::Tool::NAND_GATE:
					// ����ŵ��������������
					m = !(output[i] && output[i - 1]);
					output[i - 1] = m;
					i--;
				    break;
			    case Component::Tool::NOR_GATE:
					// ����ŵ����������Ļ��
					n = !(output[i] || output[i - 1]);
					output[i - 1] = n;
					i--;
				    break;
			    case Component::Tool::XOR_GATE:
					// ����ŵ��������������
					o = output[i] ^ output[i - 1];
					output[i - 1] = o;
					i--;
				    break;
			    case Component::Tool::XNOR_GATE:
					// ����ŵ��������������
					p = !(output[i] ^ output[i - 1]);
					output[i - 1] = p;
					i--;
				    break;
            }

			if (current.tool == Component::Tool::BULB) {
				wxLogMessage("The bulb is %s", output[i] ? "ON" : "OFF");
			}
        }
            
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
