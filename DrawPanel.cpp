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
    Component::Tool currentTool; // ��ǰ����
    std::vector<Component> components;  // �洢�����������
    std::vector<int> selectedComponents; // �洢ѡ�е��������
    std::vector<Component> copiedComponents; // �洢���Ƶ��������
    std::vector<std::pair<wxPoint, wxPoint>> connections; // �洢�����ߵ��������
    bool dragging; // ����Ƿ������϶����
    bool connecting;
    int draggedComponentIndex; // ���϶����������

    wxPoint dragStartPos;//�洢�϶���ʼλ�ã�ÿ���϶������£������ڸ�������

    wxPoint componentOffset; // ��������¼Ԫ�����������ƫ��
    //wxPoint startPoint; // ������ʼ��
    wxPoint m_mousePos;// �洢��������λ��


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

    void OnPaint(wxPaintEvent& event) {
        wxBufferedPaintDC dc(this);
        PrepareDC(dc);
        dc.SetBackground(*wxWHITE_BRUSH);
        dc.Clear();
        dc.SetUserScale(scaleFactor, scaleFactor);

        // ���������
        DrawGrid(dc);

        Render(dc);

        if (m_mousePos.x >= 0 && m_mousePos.y >= 0) {
            // ������ɫ����ԲȦ
            dc.SetPen(wxPen(wxColour(0, 0, 255), 1));  // ������ɫ�߿�
            dc.SetBrush(wxBrush(wxColour(0, 0, 255), wxBRUSHSTYLE_TRANSPARENT));  // ����͸�����
            dc.DrawCircle(m_mousePos, 3);  // ���ư뾶Ϊ10��ԲȦ
        }
        // ��ȡ��ͼ����Ĵ�С
        wxSize size = GetSize();

        // ���ñ߿���ɫ�Ϳ��
        dc.SetPen(wxPen(*wxBLACK, 2));  // ��ɫ�߿򣬿��Ϊ2����

        // ���ƾ��α߿�
        // ע�⣺�߿��λ�úʹ�С��Ҫ�����������ӵ���
        int borderWidth = 2;  // �߿���
        int scaledWidth = size.x / scaleFactor;
        int scaledHeight = size.y / scaleFactor;

        // ���Ʊ߿�
        dc.DrawRectangle(0, 0, scaledWidth, scaledHeight);
    }


    void Render(wxDC& dc) {
        for (size_t i = 0; i < components.size(); ++i) {
            auto& component = components[i];
            component.Draw(dc, m_mousePos);  // �������
        }
        DrawConnections(dc);  // ����������
    }

    //���������
    void DrawGrid(wxDC& dc) {
        const int gridSize = 20;  // �����С��20 ����
        const int dotRadius = 2;  // �����İ뾶������Ϊ 2 ����

        dc.SetPen(wxPen(wxColour(220, 220, 220), 1));  // ����ǳ��ɫ�������߿�
        dc.SetBrush(wxBrush(wxColour(220, 220, 220)));  // ���������������ɫ��ǳ��ɫ

        // ����ˮƽ�ʹ�ֱ����������
        for (int y = 0; y < GetSize().y / scaleFactor; y += gridSize) {
            for (int x = 0; x < GetSize().x / scaleFactor; x += gridSize) {
                // ��ÿ������Ԫ�����Ļ���һ��СԲ��
                dc.DrawCircle(x, y, dotRadius);
            }
        }
    }


    void DrawConnections(wxDC& dc) {
        dc.SetPen(wxPen(*wxBLACK, 3));  // ���û�����ɫ�ʹ�ϸ
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
                componentOffset = pos - components[i].position;  // ������������λ�õ�ƫ������ע�����λ����Ϊ������λ�ã������λ�ò�һ�������λ�ã��Ƿ�Χ������һ�㣩
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
            wxPoint newComponentPos = pos - componentOffset;//���λ����Ԫ��λ�ã����ģ����һ��Offsetƫ����
            wxPoint prepos = components[draggedComponentIndex].position;
            // �����������������ӵ�����
            UpdateConnections(draggedComponentIndex, pos - dragStartPos, prepos);

            dragStartPos = pos;

            // �����������λ��
            components[draggedComponentIndex].position = newComponentPos;


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

    void UpdateConnections(int draggedComponentIndex, const wxPoint& offset,wxPoint prepos) {
        // ������ǰԪ�����ڵ����е�����
        for (auto& connection : connections) {
            for (const auto& pin : components[draggedComponentIndex].pins) {
                wxPoint currentPinPosition = pin.first + prepos;
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
        wxPoint pos = GetSnapPoint(event.GetPosition());
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
            //const int connectId = wxNewId();
            menu.Append(deleteId, "Delete");
            //menu.Append(connectId, "Connect");

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

    // ȫѡ�������
    void SelectAll() {
        for (auto& component : components) {
            component.isSelected = true;  // ����������Ϊѡ��״̬
        }
        Refresh();  // ˢ����������»���
    }


    // ɾ��ѡ�е����
    void CutSelected() {
        // ʹ�� erase-remove ���÷�ɾ��ѡ�е����
        components.erase(
            std::remove_if(
                components.begin(),
                components.end(),
                [](const Component& component) {
                    return component.isSelected;  // ɾ�� isSelected Ϊ true �����
                }
            ),
            components.end()
        );
        Refresh();  // ˢ����������»���
    }

    // ����ѡ�е����
    void CopySelected() {
        copiedComponents.clear();  // ���֮ǰ�ĸ�������
        for (const auto& component : components) {
            if (component.isSelected) {
                copiedComponents.push_back(component);  // ����ѡ�е����
            }
        }
    }

    // ճ�����Ƶ����
    void PasteCopied() {
        if (copiedComponents.empty()) {
            return;  // ���û�и��Ƶ������ֱ�ӷ���
        }

        // ����ƫ������ʹճ���������ʾ��ԭʼ������Ա�
        const wxPoint offset(40, 40);  // ƫ����Ϊ (40, 40)

        for (const auto& copiedComponent : copiedComponents) {
            Component newComponent = copiedComponent;  // �������
            newComponent.position += offset;  // ƫ�����λ��
            newComponent.isSelected = false;  // ȡ��ѡ��״̬
            components.push_back(newComponent);  // ��ӵ�����б�
        }

        Refresh();  // ˢ����������»���
    }

};