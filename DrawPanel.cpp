#include<wx/wx.h>
#include <wx/toolbar.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/artprov.h>
#include <vector>
#include <wx/dcbuffer.h>
#include <wx/frame.h>       // ������ܴ�����ع���
#include <wx/treectrl.h>    // �������ؼ�����ع���
#include <fstream>
#include <nlohmann/json.hpp>
#include <wx/dc.h>
#include"Component.cpp"
//#include <wx/pen.h>

using json = nlohmann::json;

class DrawPanel : public wxPanel {
    friend class MyFrame; // ���� MyFrame Ϊ��Ԫ��,ʹ��wxframe�ܹ�����DrawPanel��˽�з���
public:
    //enum class Tool { NONE, AND_GATE, OR_GATE, NOT_GATE, NAND_GATE, NOR_GATE, XOR_GATE, XNOR_GATE, }; // ���幤�����ͣ������޹��ߡ����š����źͷ��� // ���幤�����ͣ������޹��ߡ����š����źͷ���
    wxBitmap* bitmap = nullptr;// ����λͼָ��
    wxTimer* moveTimer;
    int connectionStartIndex;
    double scaleFactor = 0.5;//��С��������ÿ��ӳ��ʱ���Ը�����

    DrawPanel(wxWindow* parent)
        : wxPanel(parent), currentTool(Component::Tool::NONE), dragging(false), moveTimer(new wxTimer(this)), connecting(false), startPoint(wxPoint(-1, -1)), connectionStartIndex(-1) {
        // ���캯������ʼ����弰������ɫ
        SetBackgroundColour(*wxWHITE);
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        bitmap = new wxBitmap(GetSize()); // ��ʼ��λͼ
        // ���¼�
        Bind(wxEVT_PAINT, &DrawPanel::OnPaint, this); // �����¼�
        Bind(wxEVT_LEFT_DOWN, &DrawPanel::OnLeftDown, this); // ��������¼�
        Bind(wxEVT_LEFT_UP, &DrawPanel::OnLeftUp, this); // ���̧���¼�
        Bind(wxEVT_MOTION, &DrawPanel::OnMouseMove, this); // ����ƶ��¼�
        Bind(wxEVT_RIGHT_DOWN, &DrawPanel::OnRightDown, this); // �Ҽ������¼�
        Bind(wxEVT_SIZE, &DrawPanel::OnSize, this); // ����С�仯�¼�
        Bind(wxEVT_TIMER, &DrawPanel::OnMoveTimer, this);// �󶨶�ʱ���¼�

    }

    ~DrawPanel() {
        delete bitmap; // �ͷ�λͼ�ڴ�
        delete moveTimer; // �ͷŶ�ʱ���ڴ�

    }

    void SetCurrentTool(Component::Tool tool) {
        currentTool = tool; // ���õ�ǰѡ��Ĺ���
    }

    //private:
    Component::Tool currentTool; // ��ǰ����
    //std::vector<std::pair<Tool, wxPoint>> components; // �洢����ӵ��������λ��
    std::vector<Component> components;  // �洢�����������
    //std::vector<int> selectedComponents;// �洢ѡ�е����
    //std::vector<std::pair<Tool, wxPoint>> copiedComponents; // �洢���Ƶ����
    std::vector<std::pair<int, int>> connections; // �洢�����ߵ��������
    bool dragging; // ����Ƿ������϶����
    bool connecting;
    int draggedComponentIndex; // ���϶����������
    wxPoint dragStartPos;
    wxPoint componentOffset; // ��������¼Ԫ�����������ƫ��
    wxPoint startPoint; // ������ʼ��
    wxPoint m_mousePos;// �洢��������λ��



    void OnPaint(wxPaintEvent& event) {
        wxBufferedPaintDC dc(this);
        PrepareDC(dc);
        dc.SetBackground(*wxWHITE_BRUSH);
        dc.Clear();
        dc.SetUserScale(scaleFactor, scaleFactor);
        Render(dc);

        if (m_mousePos.x >= 0 && m_mousePos.y >= 0) {
            // ������ɫ����ԲȦ
            dc.SetPen(wxPen(wxColour(0, 0, 255), 1));  // ������ɫ�߿�
            dc.SetBrush(wxBrush(wxColour(0, 0, 255), wxBRUSHSTYLE_TRANSPARENT));  // ����͸�����
            dc.DrawCircle(m_mousePos, 3);  // ���ư뾶Ϊ10��ԲȦ
        }
    }

    //void Render(wxDC& dc) {
    //    //DrawGrid(dc);  // ��������
    //    for (size_t i = 0; i < components.size(); ++i) {
    //        const auto& component = components[i];
    //        DrawComponent(dc, component.first, component.second, i);
    //    }
    //    DrawConnections(dc);  // ����������
    //}
    void Render(wxDC& dc) {
        for (size_t i = 0; i < components.size(); ++i) {
            auto& component = components[i];
            wxPoint snapPoint = GetSnapPoint(component.position);
            component.Draw(dc, snapPoint, m_mousePos);  // ʹ�� Component ��� Draw ����������
        }
        DrawConnections(dc);  // ����������
    }



    /*void DrawGrid(wxDC& dc) {
        dc.SetPen(wxPen(wxColour(200, 200, 200), 1, wxPENSTYLE_DOT));
        for (int i = 0; i < GetSize().GetWidth(); i += 20) {
            dc.DrawLine(i, 0, i, GetSize().GetHeight());
        }
        for (int j = 0; j < GetSize().GetHeight(); j += 20) {
            dc.DrawLine(0, j, GetSize().GetWidth(), j);
        }
    }*/
    
    void DrawComponent(wxDC& dc, const Component& component, size_t index) {
        int gridX = (component.position.x / 20) * 20;
        int gridY = (component.position.y / 20) * 20;
        wxPoint snapPoint(gridX, gridY);

        // ѡ�� JSON �ļ�������ͼ�Σ�ʡ�Դ��룩

        // ������������
        dc.SetPen(wxPen(*wxBLACK, 4));
        for (const auto& input : component.inputs) {
            dc.DrawLine(snapPoint.x + input.first.x, snapPoint.y + input.first.y,
                snapPoint.x + input.second.x, snapPoint.y + input.second.y);
        }

        // �����������
        for (const auto& output : component.outputs) {
            dc.DrawLine(snapPoint.x + output.first.x, snapPoint.y + output.first.y,
                snapPoint.x + output.second.x, snapPoint.y + output.second.y);
        }
    }


    //void DrawComponent(wxDC& dc, Tool tool, const wxPoint& pos, size_t index) {
    //    // ��λ��ӳ�䵽����
    //    int gridX = (pos.x / 20) * 20/scaleFactor;
    //    int gridY = (pos.y / 20) * 20/scaleFactor;
    //    wxPoint snapPoint(gridX, gridY);

    //    // ���ݹ�������ѡ���Ӧ�� JSON �ļ�
    //    std::string toolName;
    //    switch (tool) {
    //    case Tool::AND_GATE:
    //        toolName = "AND_GATE.json";
    //        break;
    //    case Tool::OR_GATE:
    //        toolName = "OR_GATE.json";
    //        break;
    //    case Tool::NOT_GATE:
    //        toolName = "NOT_GATE.json";
    //        break;
    //    case Tool::NAND_GATE:
    //        toolName = "NAND_GATE.json";
    //        break;
    //    case Tool::NOR_GATE:
    //        toolName = "NOR_GATE.json";
    //        break;
    //    case Tool::XOR_GATE:
    //        toolName = "XOR_GATE.json";
    //        break;
    //    case Tool::XNOR_GATE:
    //        toolName = "XNOR_GATE.json";
    //        break;
    //    default:
    //        return;
    //    }

    //    // ���ض�Ӧ�� JSON �ļ�
    //    std::ifstream file("tools/" + toolName);
    //    if (!file.is_open()) {
    //        wxLogError("Cannot open the JSON file for %s", toolName);
    //        return;
    //    }

    //    json componentJson;
    //    file >> componentJson;
    //    file.close();

    //    // ��¼�Ƿ��������������ֱ�����
    //    bool highlightInput = false;
    //    bool highlightOutput = false;

    //    // ����ֱ�߲���
    //    dc.SetPen(wxPen(*wxBLACK,4)); // ���ñ߿���ɫ�Ϳ��
    //    int i = 0;
    //    for (const auto& line : componentJson["lines"]) {
    //        i++;
    //        wxPoint start(line["start"][0].get<int>(), line["start"][1].get<int>());
    //        wxPoint end(line["end"][0].get<int>(), line["end"][1].get<int>());
    //        dc.DrawLine(snapPoint.x + start.x, snapPoint.y + start.y, snapPoint.x + end.x, snapPoint.y + end.y);
    //    }
    //    // �������߲��֣����У�
    //    if (componentJson.contains("splines")) {
    //        for (const auto& spline : componentJson["splines"]) {
    //            std::vector<wxPoint> points;
    //            for (const auto& point : spline["points"]) {
    //                points.push_back(wxPoint(snapPoint.x + point[0].get<int>(), snapPoint.y + point[1].get<int>()));
    //            }
    //            dc.DrawSpline(points.size(), points.data());
    //        }
    //    }

    //    // ����Բ�β��֣����У�
    //    if (componentJson.contains("circle")) {
    //        auto circle = componentJson["circle"];
    //        int cx = circle["center"][0].get<int>();
    //        int cy = circle["center"][1].get<int>();
    //        int radius = circle["radius"].get<int>();
    //        dc.DrawCircle(snapPoint.x + cx, snapPoint.y + cy, radius);
    //    }

    //    // ���������
    //    if (componentJson.contains("inputs")) {
    //        for (const auto& input : componentJson["inputs"]) {
    //            wxPoint start(input["start"][0].get<int>(), input["start"][1].get<int>());
    //            wxPoint end(input["end"][0].get<int>(), input["end"][1].get<int>());
    //            // �������Ƿ��������
    //            bool isMouseOver = IsMouseOverLine(start + snapPoint, end + snapPoint, m_mousePos);

    //            // ���û�����ʽ��������������ˣ�ʹ�ú�ɫ����������ʹ��Ĭ�Ϻ�ɫ
    //            if (isMouseOver) {
    //                dc.SetPen(wxPen(wxColour(255, 0, 0), 7));
    //                highlightInput = true;
    //            }
    //            else {
    //                dc.SetPen(wxPen(*wxBLACK, 4));
    //            }

    //            // ����������߶�
    //            dc.DrawLine(snapPoint.x + start.x, snapPoint.y + start.y, snapPoint.x + end.x, snapPoint.y + end.y);
    //        }
    //        // �ָ�Ĭ��������ʽ
    //        dc.SetPen(wxPen(*wxBLACK, 4));
    //    }


    //    // ���������
    //    if (componentJson.contains("outputs")) {
    //        for (const auto& output : componentJson["outputs"]) {
    //            wxPoint start(output["start"][0].get<int>(), output["start"][1].get<int>());
    //            wxPoint end(output["end"][0].get<int>(), output["end"][1].get<int>());

    //            // �������Ƿ��������
    //            if (IsMouseOverLine(start + snapPoint, end + snapPoint, m_mousePos)) {
    //                dc.SetPen(wxPen(wxColour(255, 0, 0), 7)); // ��ɫ����ϸ7
    //                highlightOutput = true;
    //            }
    //            else {
    //                dc.SetPen(wxPen(*wxBLACK, 4)); // Ĭ�Ϻ�ɫ����ϸ2
    //            }

    //            // ����������߶�
    //            dc.DrawLine(snapPoint.x + start.x, snapPoint.y + start.y, snapPoint.x + end.x, snapPoint.y + end.y);
    //        }
    //    }
    //    // �ָ�Ĭ��������ʽ
    //    dc.SetPen(wxPen(*wxBLACK, 4));
    //}

    //bool IsMouseOverLine(const wxPoint& start, const wxPoint& end, const wxPoint& mousePos) {
    //    // �����߶εľ�������
    //    int minX = std::min(start.x, end.x);
    //    int maxX = std::max(start.x, end.x);
    //    int minY = std::min(start.y, end.y);
    //    int maxY = std::max(start.y, end.y);

    //    // �ж�����Ƿ��ڸ������ڣ�����һ�������

    //    return (mousePos.x >= minX - 5 && mousePos.x <= maxX + 5 &&
    //        mousePos.y >= minY - 5 && mousePos.y <= maxY + 5);
    //}

    void DrawConnections(wxDC& dc) {
        dc.SetPen(wxPen(*wxBLUE, 2));  // ���û�����ɫ�ʹ�ϸ
        for (const auto& connection : connections) {
            // ��ȡ��ʼ����ĵ�һ������
            wxPoint startPin = components[connection.first].GetPinPosition(0, false); // ��ȡ��һ���������
            // ��ȡĿ������ĵ�һ������
            wxPoint endPin = components[connection.second].GetPinPosition(0, true);  // ��ȡ��һ����������
            dc.DrawLine(startPin, endPin);  // ����������
        }
    }



    void DrawGridAlignedLine(wxDC& dc, const wxPoint& start, const wxPoint& end) {
        wxPoint gridStart((start.x / 20) * 20, (start.y / 20) * 20);
        wxPoint gridEnd((end.x / 20) * 20, (end.y / 20) * 20);

        if (gridStart.x == gridEnd.x || gridStart.y == gridEnd.y) {
            dc.DrawLine(gridStart, gridEnd);
        }
        else {
            wxPoint midPoint(gridEnd.x, gridStart.y);
            dc.DrawLine(gridStart, midPoint);
            dc.DrawLine(midPoint, gridEnd);
        }
    }

    void OnLeftDown(wxMouseEvent& event) {
        wxPoint pos = event.GetPosition();

        // ���������������
        if (connecting) {
            for (size_t i = 0; i < components.size(); ++i) {
                // ��������������������Ż���������ϣ���¼������ʼ����ֹ�����
                if (components[i].IsMouseOverPin(pos)) {
                    if (connectionStartIndex == -1) {
                        connectionStartIndex = i;  // ��¼������ʼ���
                    }
                    else {
                        connections.emplace_back(connectionStartIndex, i);  // ��¼���ӹ�ϵ
                        connectionStartIndex = -1;  // ����������ʼ���
                    }
                    Refresh();
                    return;
                }
            }
        }

        // �ж��Ƿ��������������׼����ק
        for (size_t i = 0; i < components.size(); ++i) {
            if (components[i].IsMouseOverComponent(pos)) {
                // ��¼��ק���������ʼλ��
                dragging = true;
                draggedComponentIndex = i;
                dragStartPos = pos;
                componentOffset = pos - components[i].position;
                CaptureMouse();
                return;
            }
        }

        // ���������ǿհ����򣬴����µ����
        if (!dragging && !connecting) {
            // �����ʱѡ���� AND_GATE ��Ϊ�����
            Component::Tool newTool = Component::Tool::AND_GATE;
            wxPoint newPos = pos;  // ���������ʼλ��Ϊ���λ��
            Component newComponent(newTool, newPos);

            // �� JSON �ļ����������������Ϣ
            std::string componentFileName = newComponent.GetComponentFileName(newTool);
            newComponent.LoadPinsFromJson("tools/" + componentFileName);

            // ����������뵽����б���
            components.push_back(newComponent);

            Refresh();
        }
    }


    void OnLeftUp(wxMouseEvent& event) {
        // �ͷ��϶����
        if (dragging) {
            dragging = false;
            ReleaseMouse(); // �ͷ���겶��
        }
    }

    void OnMouseMove(wxMouseEvent& event) {
        if (dragging) {
            wxPoint pos = event.GetPosition();
            wxPoint newComponentPos = pos - componentOffset;
            components[draggedComponentIndex].position = newComponentPos;
            // ���������ߵ�λ��
            UpdateConnections(draggedComponentIndex, newComponentPos - components[draggedComponentIndex].position);
            Refresh();
        }

        wxPoint mousePos = event.GetPosition();
        mousePos.x /= scaleFactor;
        mousePos.y /= scaleFactor;
        m_mousePos = wxPoint(mousePos.x / 20 * 20, mousePos.y / 20 * 20);  // �������
        Refresh();
    }



    /*std::string GetComponentFileName(Component::Tool tool) {
        switch (tool) {
        case Tool::AND_GATE: return "AND_GATE.json";
        case Tool::OR_GATE: return "OR_GATE.json";
        case Tool::NOT_GATE: return "NOT_GATE.json";
        case Tool::NAND_GATE: return "NAND_GATE.json";
        case Tool::NOR_GATE: return "NOR_GATE.json";
        case Tool::XOR_GATE: return "XOR_GATE.json";
        case Tool::XNOR_GATE: return "XNOR_GATE.json";
        default: return "";
        }
    }*/

    wxPoint GetSnapPoint(const wxPoint& pos) {
        // ��λ��ӳ�䵽������ (�����СΪ 20)
        int gridX = (pos.x / 20) * 20;
        int gridY = (pos.y / 20) * 20;
        return wxPoint(gridX, gridY);  // ���ض����ĵ�
    }

    void UpdateConnections(int index, const wxPoint& offset) {
        for (auto& connection : connections) {
            if (connection.first == index || connection.second == index) {
                if (connection.first == index) {
                    components[connection.first].position += offset;
                }
                if (connection.second == index) {
                    components[connection.second].position += offset;
                }
            }
        }
    }



    void OnMoveTimer(wxTimerEvent&) {
        Refresh(); // ˢ�»�ͼ
        moveTimer->Stop(); // ֹͣ��ʱ��
    }

    void OnRightDown(wxMouseEvent& event) {
        wxPoint pos = event.GetPosition();
        bool componentFound = false;
        int componentToDelete = -1;

        for (size_t i = 0; i < components.size(); ++i) {
            if (abs(components[i].position.x - pos.x) < 20 && abs(components[i].position.y - pos.y) < 20) {
                componentFound = true;
                componentToDelete = i;
                break;
            }
        }

        if (componentFound) {
            wxMenu menu;
            menu.Append(wxID_ANY, "Delete");
            menu.Append(wxID_ANY, "Connect");

            // ɾ������
            Bind(wxEVT_MENU, [this, componentToDelete](wxCommandEvent&) {
                if (componentToDelete != -1) {
                    components.erase(components.begin() + componentToDelete);
                    Refresh();
                }
                }, wxID_ANY);

            // ���Ӳ���
            Bind(wxEVT_MENU, [this, componentToDelete](wxCommandEvent&) {
                if (componentToDelete != -1) {
                    connecting = true;
                    connectionStartIndex = componentToDelete;
                }
                }, wxID_ANY);

            PopupMenu(&menu);
        }
    }


    void OnSize(wxSizeEvent& event) {
        Refresh(); // ����С�ı�ʱˢ�»�ͼ
        event.Skip(); // �������������¼�
    }

    //// ѡ���������
    //void SelectAll() {
    //    selectedComponents.clear(); // ���֮ǰ��ѡ��
    //    for (size_t i = 0; i < components.size(); ++i) {
    //        selectedComponents.push_back(i); // ���������������ӵ�ѡ���б�
    //    }
    //    Refresh(); // ˢ�»�ͼ
    //}

    //// ɾ��ѡ�е����
    //void CutSelected() {
    //    for (auto it = selectedComponents.rbegin(); it != selectedComponents.rend(); ++it) {
    //        components.erase(components.begin() + *it); // �� components ��ɾ��
    //    }
    //    selectedComponents.clear(); // ���ѡ�е����
    //    Refresh(); // ˢ�»�ͼ
    //}

    ////����ѡ�е����
    //void CopySelected() {
    //    copiedComponents.clear(); // ���֮ǰ�ĸ�������
    //    for (int index : selectedComponents) {
    //        if (index >= 0 && index < components.size()) {
    //            copiedComponents.push_back(components[index]); // �������
    //        }
    //    }
    //}

    ////ճ��ѡ�е����
    //void PasteCopied() {
    //    for (const auto& component : copiedComponents) {
    //        // �ڸ��������λ���Ͻ���ճ������΢����λ��
    //        wxPoint newPosition = component.second + wxPoint(10, 10); // ƫ��λ��
    //        components.push_back({ component.first, newPosition }); // ��ӵ�����б���
    //    }
    //    Refresh(); // ˢ�»�ͼ
    //}

};