#include <wx/wx.h>
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
    wxBitmap* bitmap = nullptr;// ����λͼָ��
    wxTimer* moveTimer;
    wxPoint connectionStartPosition;
    double scaleFactor = 0.5;//��С��������ÿ��ӳ��ʱ���Ը�����

    DrawPanel(wxWindow* parent)
        : wxPanel(parent), currentTool(Component::Tool::NONE), dragging(false), moveTimer(new wxTimer(this)), connecting(false), connectionStartPosition(wxPoint(-1,-1)) {
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

    Component::Tool currentTool; // ��ǰ����
    std::vector<Component> components;  // �洢�����������
    std::vector<std::pair<wxPoint, wxPoint>> connections; // �洢�����ߵ��������
    bool dragging; // ����Ƿ������϶����
    bool connecting;
    int draggedComponentIndex; // ���϶����������
    wxPoint dragStartPos;
    wxPoint componentOffset; // ��������¼Ԫ�����������ƫ��
    //wxPoint startPoint; // ������ʼ��
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


    void Render(wxDC& dc) {
        for (size_t i = 0; i < components.size(); ++i) {
            auto& component = components[i];
            component.Draw(dc, m_mousePos);  // ʹ�� Component ��� Draw ����������
        }
        DrawConnections(dc);  // ����������
    }


    void DrawConnections(wxDC& dc) {
        dc.SetPen(wxPen(*wxBLUE, 2));  // ���û�����ɫ�ʹ�ϸ
        for (const auto& connection : connections) {
            // ��ȡ��ʼ����ĵ�һ������
            wxPoint startPin = connection.first; // ��ȡ��һ���������
            // ��ȡĿ������ĵ�һ������
            wxPoint endPin = connection.second;  // ��ȡ��һ����������
            DrawGridAlignedLine(dc,startPin, endPin);  // ����������
        }
    }



    void DrawGridAlignedLine(wxDC& dc, const wxPoint& start, const wxPoint& end) {
        // ������뵽����������յ�
        wxPoint gridStart((start.x / 20) * 20, (start.y / 20) * 20);
        wxPoint gridEnd((end.x / 20) * 20, (end.y / 20) * 20);

        // ��� x ������ͬ��˵�����ߴ�ֱ
        if (gridStart.x == gridEnd.x) {
            dc.DrawLine(gridStart, gridEnd);
        }
        // ��� y ������ͬ��˵������ˮƽ
        else if (gridStart.y == gridEnd.y) {
            dc.DrawLine(gridStart, gridEnd);
        }
        else {
            // ����������������ߣ���ˮƽ�ٴ�ֱ
            wxPoint midPoint(gridEnd.x, gridStart.y);  // ˮƽ����ֱ�����ӵ�
            dc.DrawLine(gridStart, midPoint);
            dc.DrawLine(midPoint, gridEnd);
        }
    }


    void OnLeftDown(wxMouseEvent& event) {
        wxPoint pos = GetSnapPoint(event.GetPosition());
        // ���������������
        if (connecting) {
            for (size_t i = 0; i < components.size(); ++i) {
                // ��������������������Ż���������ϣ���¼������ʼ����ֹ�����
                if (components[i].IsMouseOverPin(pos)) {
                    if (connectionStartPosition == wxPoint(-1,-1)) {
                        connectionStartPosition = components[i].GetPinPosition(pos);
                    }
                    else {
                        wxPoint pinPosition = components[i].GetPinPosition(pos);
                        connections.push_back(std::pair<wxPoint, wxPoint>(connectionStartPosition, pinPosition));
                        connectionStartPosition = wxPoint(-1, -1);  // ����������ʼ���
                        connecting = false;
                    }
                    Refresh();
                    return;
                }
            }
        }

        // �ж��Ƿ��������������׼����ק
        for (size_t i = 0; i < components.size(); ++i) {
            if (components[i].IsMouseOverComponent(pos)){
                // ��¼��ק���������ʼλ��
                dragging = true;
                draggedComponentIndex = i;
                dragStartPos = pos;
                componentOffset = pos - components[i].position;  // ������������λ�õ�ƫ����
                CaptureMouse();
                return;
            }
        }

        // ���������ǿհ����򣬴����µ����
        if (!dragging && !connecting) {
            // �����ʱѡ���� AND_GATE ��Ϊ�����
            wxPoint newPos = pos;  // ���������ʼλ��Ϊ���λ��
            Component newComponent(currentTool, newPos);

            // �� JSON �ļ����������������Ϣ
            std::string componentFileName = newComponent.GetComponentFileName(currentTool);
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
            wxPoint pos = GetSnapPoint(event.GetPosition());
            wxPoint newComponentPos = pos - componentOffset;

            // �����������λ��
            components[draggedComponentIndex].position = newComponentPos;

            // �����������������ӵ�����
            UpdateConnections(draggedComponentIndex, pos - dragStartPos);

            // ˢ�»���
            Refresh();
        }

        m_mousePos = GetSnapPoint(event.GetPosition());
        Refresh();
    }


    wxPoint GetSnapPoint(const wxPoint& pos) {
        // ��λ��ӳ�䵽������ (�����СΪ 20)
        wxPoint temp = pos;
        temp.x /= scaleFactor;
        temp.y /= scaleFactor;
        int gridX = (temp.x / 20) * 20;
        int gridY = (temp.y / 20) * 20;

        return wxPoint(gridX, gridY);  // ���ض����ĵ�
    }

    void UpdateConnections(int draggedComponentIndex, const wxPoint& offset) {
        // �������е�����
        for (auto& connection : connections) {
            for (const auto& pin : components[draggedComponentIndex].pins) {
                wxPoint currentPinPosition = pin.first + components[draggedComponentIndex].position;
                if (connection.first == currentPinPosition) {
                    connection.first += offset;
                }
                if (connection.second == currentPinPosition) {
                    connection.second += offset;
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
        int componentSelected = -1;

        for (size_t i = 0; i < components.size(); ++i) {
            if (components[i].IsMouseOverComponent(pos)) {
                componentFound = true;
                componentSelected = i;
                break;
            }
        }

        if (componentFound) {
            wxMenu menu;
            
            const int deleteId = wxNewId();
            const int connectId = wxNewId();
            menu.Append(deleteId, "Delete");
            menu.Append(connectId, "Connect");

            // ɾ������
            Bind(wxEVT_MENU, [this, componentSelected](wxCommandEvent&) {
                if (componentSelected != -1) {
                    components.erase(components.begin() + componentSelected);
                    Refresh();
                }
                }, deleteId);

            PopupMenu(&menu);
        }
    }


    void OnSize(wxSizeEvent& event) {
        Refresh(); // ����С�ı�ʱˢ�»�ͼ
        event.Skip(); // �������������¼�
    }

};