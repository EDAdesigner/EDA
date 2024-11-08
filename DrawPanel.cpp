#include<wx/wx.h>
#include <wx/toolbar.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/artprov.h>
#include <vector>
#include <wx/dcbuffer.h>
#include <wx/frame.h>       // ������ܴ�����ع���
#include <wx/treectrl.h>    // �������ؼ�����ع���


class DrawPanel : public wxPanel {
    friend class MyFrame; // ���� MyFrame Ϊ��Ԫ��,ʹ��wxframe�ܹ�����DrawPanel��˽�з���
public:
    enum class Tool { NONE, AND_GATE, OR_GATE, NOT_GATE, NAND_GATE, NOR_GATE, XOR_GATE, XNOR_GATE, }; // ���幤�����ͣ������޹��ߡ����š����źͷ��� // ���幤�����ͣ������޹��ߡ����š����źͷ���
    wxBitmap* bitmap = nullptr;// ����λͼָ��
    wxTimer* moveTimer;


    DrawPanel(wxWindow* parent)
        : wxPanel(parent), currentTool(Tool::NONE), dragging(false), moveTimer(new wxTimer(this)), connecting(false), startPoint(wxPoint(-1, -1)), connectionStartIndex(-1) {
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

    void SetCurrentTool(Tool tool) {
        currentTool = tool; // ���õ�ǰѡ��Ĺ���
    }

    //private:
    Tool currentTool; // ��ǰ����
    std::vector<std::pair<Tool, wxPoint>> components; // �洢����ӵ��������λ��
    std::vector<int> selectedComponents;// �洢ѡ�е����
    std::vector<std::pair<Tool, wxPoint>> copiedComponents; // �洢���Ƶ����
    std::vector<std::pair<int, int>> connections; // �洢�����ߵ��������
    bool dragging; // ����Ƿ������϶����
    bool connecting;
    int draggedComponentIndex; // ���϶����������
    wxPoint dragStartPos;
    wxPoint componentOffset; // ��������¼Ԫ�����������ƫ��
    wxPoint startPoint; // ������ʼ��
    int connectionStartIndex; // ��������

    void OnPaint(wxPaintEvent& event) {
        wxBufferedPaintDC dc(this);
        PrepareDC(dc);
        dc.SetBackground(*wxWHITE_BRUSH);
        dc.Clear();
        Render(dc);
    }

    void Render(wxDC& dc) {
        DrawGrid(dc);
        for (const auto& component : components) {
            DrawComponent(dc, component.first, component.second);
        }
        DrawConnections(dc);

    }

    void DrawGrid(wxDC& dc) {
        dc.SetPen(wxPen(wxColour(200, 200, 200), 1, wxPENSTYLE_DOT));
        for (int i = 0; i < GetSize().GetWidth(); i += 20) {
            dc.DrawLine(i, 0, i, GetSize().GetHeight());
        }
        for (int j = 0; j < GetSize().GetHeight(); j += 20) {
            dc.DrawLine(0, j, GetSize().GetWidth(), j);
        }
    }

    void DrawComponent(wxDC& dc, Tool tool, const wxPoint& pos) {
        int gridX = (pos.x / 20) * 20;
        int gridY = (pos.y / 20) * 20;
        wxPoint snapPoint(gridX, gridY);
        // ���ݹ������ͻ��ƶ�Ӧ�����
        if (tool == Tool::AND_GATE) {
            dc.SetPen(wxPen(*wxBLACK, 4)); // �߿���ɫ�Ϳ��
            //��������ֱ�߲���
            dc.DrawLine(snapPoint.x - 20, snapPoint.y - 20, snapPoint.x, snapPoint.y - 20);
            dc.DrawLine(snapPoint.x - 20, snapPoint.y - 20, snapPoint.x - 20, snapPoint.y + 20);
            dc.DrawLine(snapPoint.x - 20, snapPoint.y + 20, snapPoint.x, snapPoint.y + 20);
            //��������Բ������
            wxPoint points[5] = {
                wxPoint(snapPoint.x, snapPoint.y - 20),
                wxPoint(snapPoint.x + 10, snapPoint.y - 17),
                wxPoint(snapPoint.x + 20, snapPoint.y),
                wxPoint(snapPoint.x + 10, snapPoint.y + 17),
                wxPoint(snapPoint.x, snapPoint.y + 20),
            };
            dc.DrawSpline(5, points);
            //�������������
            dc.DrawLine(snapPoint.x - 20, snapPoint.y + 10, snapPoint.x - 27, snapPoint.y + 10);
            dc.DrawLine(snapPoint.x - 20, snapPoint.y - 10, snapPoint.x - 27, snapPoint.y - 10);
            dc.DrawLine(snapPoint.x + 20, snapPoint.y, snapPoint.x + 25, snapPoint.y);
        }
        else if (tool == Tool::OR_GATE) {
            dc.SetPen(wxPen(*wxBLACK, 4)); // �߿���ɫ�Ϳ��;
            //���ƻ�����ಿ��
            wxPoint leftPoints[3] = {
                wxPoint(snapPoint.x - 20, snapPoint.y - 20),
                wxPoint(snapPoint.x - 10, snapPoint.y),
                wxPoint(snapPoint.x - 20, snapPoint.y + 20),
            };
            dc.DrawSpline(3, leftPoints);
            //���ƻ����Ҳಿ��
            wxPoint rightPoints[5] = {
                wxPoint(snapPoint.x - 20, snapPoint.y - 20),
                wxPoint(snapPoint.x, snapPoint.y - 18),
                wxPoint(snapPoint.x + 25, snapPoint.y),
                wxPoint(snapPoint.x, snapPoint.y + 18),
                wxPoint(snapPoint.x - 20, snapPoint.y + 20)
            };
            dc.DrawSpline(5, rightPoints);
            //�������������
            dc.DrawLine(snapPoint.x - 14, snapPoint.y + 10, snapPoint.x - 25, snapPoint.y + 10);
            dc.DrawLine(snapPoint.x - 14, snapPoint.y - 10, snapPoint.x - 25, snapPoint.y - 10);
            dc.DrawLine(snapPoint.x + 20, snapPoint.y, snapPoint.x + 25, snapPoint.y);
        }
        else if (tool == Tool::NOT_GATE) {
            dc.SetPen(wxPen(*wxBLACK, 4)); // �߿���ɫ�Ϳ��;
            //���Ʒ�����ಿ��
            dc.DrawLine(snapPoint.x - 20, snapPoint.y - 20, snapPoint.x + 12, snapPoint.y);
            dc.DrawLine(snapPoint.x - 20, snapPoint.y + 20, snapPoint.x - 20, snapPoint.y - 20);
            dc.DrawLine(snapPoint.x - 20, snapPoint.y + 20, snapPoint.x + 12, snapPoint.y);
            //���Ʒ����Ҳಿ��
            dc.DrawCircle(snapPoint.x + 16, snapPoint.y, 4);
            //�������������
            dc.DrawLine(snapPoint.x - 20, snapPoint.y, snapPoint.x - 27, snapPoint.y);
            dc.DrawLine(snapPoint.x + 20, snapPoint.y, snapPoint.x + 25, snapPoint.y);
        }
        else if (tool == Tool::NAND_GATE) {
            dc.SetPen(wxPen(*wxBLACK, 4)); // �߿���ɫ�Ϳ��;
            //������������ֱ�߲���
            dc.DrawLine(snapPoint.x - 20, snapPoint.y - 20, snapPoint.x - 4, snapPoint.y - 20);
            dc.DrawLine(snapPoint.x - 20, snapPoint.y - 20, snapPoint.x - 20, snapPoint.y + 20);
            dc.DrawLine(snapPoint.x - 20, snapPoint.y + 20, snapPoint.x - 4, snapPoint.y + 20);
            //���������Բ������
            wxPoint points[5] = {
                wxPoint(snapPoint.x - 4, snapPoint.y - 20),
                wxPoint(snapPoint.x + 6, snapPoint.y - 17),
                wxPoint(snapPoint.x + 16, snapPoint.y),
                wxPoint(snapPoint.x + 6, snapPoint.y + 17),
                wxPoint(snapPoint.x - 4, snapPoint.y + 20),
            };
            dc.DrawSpline(5, points);
            //����������Ҳಿ��
            dc.DrawCircle(snapPoint.x + 16, snapPoint.y, 4);
            //�������������
            dc.DrawLine(snapPoint.x - 20, snapPoint.y + 10, snapPoint.x - 27, snapPoint.y + 10);
            dc.DrawLine(snapPoint.x - 20, snapPoint.y - 10, snapPoint.x - 27, snapPoint.y - 10);
            dc.DrawLine(snapPoint.x + 20, snapPoint.y, snapPoint.x + 25, snapPoint.y);
        }
        else if (tool == Tool::NOR_GATE) {
            dc.SetPen(wxPen(*wxBLACK, 4)); // �߿���ɫ�Ϳ��;
            //���ƻ������໡�߲���
            wxPoint leftPoints[3] = {
                wxPoint(snapPoint.x - 20, snapPoint.y - 20),
                wxPoint(snapPoint.x - 10, snapPoint.y),
                wxPoint(snapPoint.x - 20, snapPoint.y + 20),
            };
            dc.DrawSpline(3, leftPoints);
            //���ƻ�����Ҳ໡�߲���
            wxPoint rightPoints[5] = {
                wxPoint(snapPoint.x - 20, snapPoint.y - 20),
                wxPoint(snapPoint.x, snapPoint.y - 18),
                wxPoint(snapPoint.x + 18, snapPoint.y),
                wxPoint(snapPoint.x, snapPoint.y + 18),
                wxPoint(snapPoint.x - 20, snapPoint.y + 20)
            };
            dc.DrawSpline(5, rightPoints);
            //���ƻ����ԲȦ����
            dc.DrawCircle(snapPoint.x + 16, snapPoint.y, 4);
            //�������������
            dc.DrawLine(snapPoint.x - 14, snapPoint.y + 10, snapPoint.x - 25, snapPoint.y + 10);
            dc.DrawLine(snapPoint.x - 14, snapPoint.y - 10, snapPoint.x - 25, snapPoint.y - 10);
            dc.DrawLine(snapPoint.x + 20, snapPoint.y, snapPoint.x + 25, snapPoint.y);
        }
        else if (tool == Tool::XOR_GATE) {
            dc.SetPen(wxPen(*wxBLACK, 4)); // �߿���ɫ�Ϳ��;
            //�����������໡�߲���
            wxPoint leftPoints[3] = {
                wxPoint(snapPoint.x - 20, snapPoint.y - 20),
                wxPoint(snapPoint.x - 10, snapPoint.y),
                wxPoint(snapPoint.x - 20, snapPoint.y + 20),
            };
            dc.DrawSpline(3, leftPoints);
            //����������м仡�߲���
            wxPoint centerPoints[3] = {
                wxPoint(snapPoint.x - 15, snapPoint.y - 20),
                wxPoint(snapPoint.x - 5, snapPoint.y),
                wxPoint(snapPoint.x - 15, snapPoint.y + 20),
            };
            dc.DrawSpline(3, centerPoints);
            //����������Ҳ໡�߲���
            wxPoint rightPoints[5] = {
                wxPoint(snapPoint.x - 15, snapPoint.y - 20),
                wxPoint(snapPoint.x, snapPoint.y - 18),
                wxPoint(snapPoint.x + 25, snapPoint.y),
                wxPoint(snapPoint.x, snapPoint.y + 18),
                wxPoint(snapPoint.x - 15, snapPoint.y + 20)
            };
            dc.DrawSpline(5, rightPoints);
            //�������������
            dc.DrawLine(snapPoint.x - 16, snapPoint.y + 10, snapPoint.x - 25, snapPoint.y + 10);
            dc.DrawLine(snapPoint.x - 16, snapPoint.y - 10, snapPoint.x - 25, snapPoint.y - 10);
            dc.DrawLine(snapPoint.x + 20, snapPoint.y, snapPoint.x + 25, snapPoint.y);
        }
        else if (tool == Tool::XNOR_GATE) {
            dc.SetPen(wxPen(*wxBLACK, 4)); // �߿���ɫ�Ϳ��;
            //����ͬ������໡�߲���
            wxPoint leftPoints[3] = {
                wxPoint(snapPoint.x - 20, snapPoint.y - 20),
                wxPoint(snapPoint.x - 10, snapPoint.y),
                wxPoint(snapPoint.x - 20, snapPoint.y + 20),
            };
            dc.DrawSpline(3, leftPoints);
            //����ͬ�����м仡�߲���
            wxPoint centerPoints[3] = {
                wxPoint(snapPoint.x - 15, snapPoint.y - 20),
                wxPoint(snapPoint.x - 5, snapPoint.y),
                wxPoint(snapPoint.x - 15, snapPoint.y + 20),
            };
            dc.DrawSpline(3, centerPoints);
            //����ͬ�����Ҳ໡�߲���
            wxPoint rightPoints[5] = {
                wxPoint(snapPoint.x - 15, snapPoint.y - 20),
                wxPoint(snapPoint.x, snapPoint.y - 18),
                wxPoint(snapPoint.x + 19, snapPoint.y),
                wxPoint(snapPoint.x, snapPoint.y + 18),
                wxPoint(snapPoint.x - 15, snapPoint.y + 20)
            };
            dc.DrawSpline(5, rightPoints);
            //����ͬ�����Ҳ�ԲȦ����
            dc.DrawCircle(snapPoint.x + 16, snapPoint.y, 4);
            //�������������
            dc.DrawLine(snapPoint.x - 16, snapPoint.y + 10, snapPoint.x - 25, snapPoint.y + 10);
            dc.DrawLine(snapPoint.x - 16, snapPoint.y - 10, snapPoint.x - 25, snapPoint.y - 10);
            dc.DrawLine(snapPoint.x + 20, snapPoint.y, snapPoint.x + 25, snapPoint.y);
        }
    }
    void DrawConnections(wxDC& dc) {
        dc.SetPen(wxPen(*wxBLUE, 2));
        for (const auto& connection : connections) {
            wxPoint start = components[connection.first].second;
            wxPoint end = components[connection.second].second;
            DrawGridAlignedLine(dc, start, end);
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

        for (size_t i = 0; i < components.size(); ++i) {
            if (abs(components[i].second.x - pos.x) < 20 && abs(components[i].second.y - pos.y) < 20) {
                if (connecting) {
                    connections.emplace_back(connectionStartIndex, i);
                    connecting = false;
                    Refresh();
                    return;
                }
                dragging = true;
                draggedComponentIndex = i;
                dragStartPos = pos;
                componentOffset = pos - components[i].second; // ��¼ƫ��
                CaptureMouse();
                return;
            }
        }

        if (!dragging && currentTool != Tool::NONE) {
            components.emplace_back(currentTool, pos);
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
            wxPoint newComponentPos = pos - componentOffset; // ʹ��ƫ�Ƽ�����λ��
            wxPoint& componentPos = components[draggedComponentIndex].second;
            componentPos = newComponentPos;

            // ����������λ��
            UpdateConnections(draggedComponentIndex, newComponentPos - componentPos);

            Refresh();
            Update();
        }
    }

    void UpdateConnections(int index, const wxPoint& offset) {
        for (auto& connection : connections) {
            if (connection.first == index || connection.second == index) {
                if (connection.first == index) {
                    components[connection.first].second += offset;
                }
                if (connection.second == index) {
                    components[connection.second].second += offset;
                }
            }
        }
    }

    // ���һ���µķ�������ʱ���¼�
    void OnMoveTimer(wxTimerEvent&) {
        Refresh(); // ˢ�»�ͼ
        moveTimer->Stop(); // ֹͣ��ʱ��
    }

    void OnRightDown(wxMouseEvent& event) {
        wxPoint pos = event.GetPosition();
        bool componentFound = false;
        int componentToDelete = -1;

        for (size_t i = 0; i < components.size(); ++i) {
            if (abs(components[i].second.x - pos.x) < 20 && abs(components[i].second.y - pos.y) < 20) {
                componentFound = true;
                componentToDelete = i;
                break;
            }
        }

        if (componentFound) {
            wxMenu menu;
            menu.Append(wxID_ANY, "Delete");
            menu.Append(wxID_ANY, "Connect");

            Bind(wxEVT_MENU, [this, componentToDelete](wxCommandEvent&) {
                if (componentToDelete != -1) {
                    components.erase(components.begin() + componentToDelete);
                    Refresh();
                }
                }, wxID_ANY);

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

    // ѡ���������
    void SelectAll() {
        selectedComponents.clear(); // ���֮ǰ��ѡ��
        for (size_t i = 0; i < components.size(); ++i) {
            selectedComponents.push_back(i); // ���������������ӵ�ѡ���б�
        }
        Refresh(); // ˢ�»�ͼ
    }

    // ɾ��ѡ�е����
    void CutSelected() {
        for (auto it = selectedComponents.rbegin(); it != selectedComponents.rend(); ++it) {
            components.erase(components.begin() + *it); // �� components ��ɾ��
        }
        selectedComponents.clear(); // ���ѡ�е����
        Refresh(); // ˢ�»�ͼ
    }

    //����ѡ�е����
    void CopySelected() {
        copiedComponents.clear(); // ���֮ǰ�ĸ�������
        for (int index : selectedComponents) {
            if (index >= 0 && index < components.size()) {
                copiedComponents.push_back(components[index]); // �������
            }
        }
    }

    //ճ��ѡ�е����
    void PasteCopied() {
        for (const auto& component : copiedComponents) {
            // �ڸ��������λ���Ͻ���ճ������΢����λ��
            wxPoint newPosition = component.second + wxPoint(10, 10); // ƫ��λ��
            components.push_back({ component.first, newPosition }); // ��ӵ�����б���
        }
        Refresh(); // ˢ�»�ͼ
    }

};