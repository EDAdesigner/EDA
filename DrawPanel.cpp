#include<wx/wx.h>
#include <wx/toolbar.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/artprov.h>
#include <vector>
#include <wx/frame.h>       // ������ܴ�����ع���
#include <wx/treectrl.h>    // �������ؼ�����ع���


class DrawPanel : public wxPanel {
    friend class MyFrame; // ���� MyFrame Ϊ��Ԫ��,ʹ��wxframe�ܹ�����DrawPanel��˽�з���
public:
    enum class Tool { NONE, AND_GATE, OR_GATE, NOT_GATE, NAND_GATE, NOR_GATE, XOR_GATE, XNOR_GATE, }; // ���幤�����ͣ������޹��ߡ����š����źͷ��� // ���幤�����ͣ������޹��ߡ����š����źͷ���
    wxBitmap* bitmap = nullptr;// ����λͼָ��

    DrawPanel(wxWindow* parent)
        : wxPanel(parent), currentTool(Tool::NONE), dragging(false) {
        // ���캯������ʼ����弰������ɫ
        SetBackgroundColour(*wxWHITE);
        bitmap = new wxBitmap(GetSize()); // ��ʼ��λͼ
        // ���¼�
        Bind(wxEVT_PAINT, &DrawPanel::OnPaint, this); // �����¼�
        Bind(wxEVT_LEFT_DOWN, &DrawPanel::OnLeftDown, this); // ��������¼�
        Bind(wxEVT_LEFT_UP, &DrawPanel::OnLeftUp, this); // ���̧���¼�
        Bind(wxEVT_MOTION, &DrawPanel::OnMouseMove, this); // ����ƶ��¼�
        Bind(wxEVT_RIGHT_DOWN, &DrawPanel::OnRightDown, this); // �Ҽ������¼�
        Bind(wxEVT_SIZE, &DrawPanel::OnSize, this); // ����С�仯�¼�
    }

    ~DrawPanel() {
        delete bitmap; // �ͷ�λͼ�ڴ�
    }

    void SetCurrentTool(Tool tool) {
        currentTool = tool; // ���õ�ǰѡ��Ĺ���
    }

    //private:
    Tool currentTool; // ��ǰ����
    std::vector<std::pair<Tool, wxPoint>> components; // �洢����ӵ��������λ��
    std::vector<int> selectedComponents;// �洢ѡ�е����
    std::vector<std::pair<Tool, wxPoint>> copiedComponents; // �洢���Ƶ����
    bool dragging; // ����Ƿ������϶����
    int draggedComponentIndex; // ���϶����������
    wxPoint dragStartPos; // �϶���ʼλ��

    void OnPaint(wxPaintEvent& event) {
        if (!bitmap || bitmap->GetSize() != GetSize()) {
            delete bitmap;  // ɾ����λͼ
            bitmap = new wxBitmap(GetSize()); // �����µ�λͼ
        }
        Render(*bitmap); // ÿ�λ��ƶ�����λͼ
        wxPaintDC dc(this);
        dc.DrawBitmap(*bitmap, 0, 0); // ��λͼ���Ƶ������
    }

    void Render(wxBitmap& bitmap) {
        wxMemoryDC memDC(bitmap); // ʹ���ڴ�DC���Ƶ�λͼ
        memDC.SetBackground(*wxWHITE_BRUSH);
        memDC.Clear();
        DrawGrid(memDC);
        for (const auto& component : components) {
            DrawComponent(memDC, component.first, component.second);
        }
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



    void OnLeftDown(wxMouseEvent& event) {
        wxPoint pos = event.GetPosition(); // ��ȡ�����λ��
        // ����Ƿ��������������
        for (size_t i = 0; i < components.size(); ++i) {
            if (abs(components[i].second.x - pos.x) < 20 && abs(components[i].second.y - pos.y) < 20) {
                dragging = true;
                draggedComponentIndex = i;
                dragStartPos = pos;
                CaptureMouse();
                return;
            }
        }

        // ���û���϶��������ѡ���˹��ߣ�����������
        if (currentTool != Tool::NONE) {
            components.emplace_back(currentTool, pos); // ������
            Refresh(); // ˢ�»�ͼ
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
        // ��������϶����
        if (dragging) {
            wxPoint pos = event.GetPosition(); // ��ȡ��ǰ���λ��
            // ����ƫ����
            wxPoint offset = pos - dragStartPos;
            // �������λ��
            components[draggedComponentIndex].second += offset;
            dragStartPos = pos; // �����϶���ʼλ��
            Refresh(); // ˢ�»�ͼ
        }
    }

    void OnRightDown(wxMouseEvent& event) {
        wxPoint pos = event.GetPosition(); // ��ȡ�����λ��
        bool componentFound = false; // ����Ƿ��ҵ����
        int componentToDelete = -1; // ��¼Ҫɾ�����������

        // ����Ƿ��������������
        for (size_t i = 0; i < components.size(); ++i) {
            if (abs(components[i].second.x - pos.x) < 20 && abs(components[i].second.y - pos.y) < 20) {
                componentFound = true; // �ҵ����
                componentToDelete = i; // ��¼�������
                break; // �˳�ѭ��
            }
        }

        // ����ҵ����������ʾɾ���˵�
        if (componentFound) {
            wxMenu menu; // ���������Ĳ˵�
            menu.Append(wxID_ANY, "Delete"); // ���ɾ��ѡ��
            // �󶨲˵�����¼�
            Bind(wxEVT_MENU, [this, componentToDelete](wxCommandEvent&) {
                if (componentToDelete != -1) {
                    components.erase(components.begin() + componentToDelete); // ɾ�����
                    Refresh(); // ˢ�»�ͼ
                }
                }, wxID_ANY); // ʹ�ð󶨵�����ID
            PopupMenu(&menu); // ��ʾ�˵�
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