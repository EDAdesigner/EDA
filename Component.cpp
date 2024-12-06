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

using json = nlohmann::json;


class Component {
public:
    enum class Tool { NONE, AND_GATE, OR_GATE, NOT_GATE, NAND_GATE, NOR_GATE, XOR_GATE, XNOR_GATE, }; // ���幤�����ͣ������޹��ߡ����š����źͷ��� // ���幤�����ͣ������޹��ߡ����š����źͷ���

    Tool tool;  // ���������
    wxPoint position;  // �����λ��
    std::vector<std::pair<wxPoint, wxPoint>> inputs;  // �������ţ������յ㣩
    std::vector<std::pair<wxPoint, wxPoint>> outputs; // ������ţ������յ㣩

    Component(Tool t, const wxPoint& pos)
        : tool(t), position(pos) {}

    // �� JSON �ļ����������������Ϣ
    void LoadPinsFromJson(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            wxLogError("Cannot open the JSON file for %s", filename);
            return;
        }

        json componentJson;
        file >> componentJson;
        file.close();

        // ��������˿�
        if (componentJson.contains("inputs")) {
            for (const auto& input : componentJson["inputs"]) {
                wxPoint start(input["start"][0].get<int>(), input["start"][1].get<int>());
                wxPoint end(input["end"][0].get<int>(), input["end"][1].get<int>());
                inputs.push_back({ start, end });
            }
        }

        // ��������˿�
        if (componentJson.contains("outputs")) {
            for (const auto& output : componentJson["outputs"]) {
                wxPoint start(output["start"][0].get<int>(), output["start"][1].get<int>());
                wxPoint end(output["end"][0].get<int>(), output["end"][1].get<int>());
                outputs.push_back({ start, end });
            }
        }
    }

    // �����������
    void Draw(wxDC& dc, const wxPoint& snapPoint, const wxPoint& mousePos) {
        // ���ݹ�������ѡ���Ӧ�� JSON �ļ�
        std::string filename = GetComponentFileName(tool);

        // ���Ƹ����
        DrawComponentShape(dc, snapPoint);

        // ���������������
        DrawPins(dc, snapPoint, mousePos);
    }

    // �ж�����Ƿ������������
    bool IsMouseOverComponent(const wxPoint& mousePos) const {
        int width = 50;  // ����������Ϊ50����
        int height = 50; // ��������߶�Ϊ50����
        return mousePos.x >= position.x && mousePos.x <= position.x + width &&
            mousePos.y >= position.y && mousePos.y <= position.y + height;
    }
    
    // �ж�����Ƿ���ĳ��������
    bool IsMouseOverPin(const wxPoint& mousePos) const {
        for (const auto& pin : inputs) {
            if (IsMouseOverLine(pin.first + position, pin.second + position, mousePos)) {
                return true;
            }
        }
        for (const auto& pin : outputs) {
            if (IsMouseOverLine(pin.first + position, pin.second + position, mousePos)) {
                return true;
            }
        }
        return false;
    }

    // �ж�����Ƿ������ŵ��߶���
    bool IsMouseOverLine(const wxPoint& start, const wxPoint& end, const wxPoint& mousePos) const {
        int minX = std::min(start.x, end.x);
        int maxX = std::max(start.x, end.x);
        int minY = std::min(start.y, end.y);
        int maxY = std::max(start.y, end.y);
        return (mousePos.x >= minX - 5 && mousePos.x <= maxX + 5 &&
            mousePos.y >= minY - 5 && mousePos.y <= maxY + 5);
    }

    std::string GetComponentFileName(Tool tool) {
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
    }

    // ��ȡָ�����ŵ���ʼλ��
    wxPoint GetPinPosition(size_t pinIndex, bool isInput) const {
        if (isInput && pinIndex < inputs.size()) {
            return inputs[pinIndex].first + position;  // �������ŵ���ʼλ��
        }
        if (!isInput && pinIndex < outputs.size()) {
            return outputs[pinIndex].first + position;  // ������ŵ���ʼλ��
        }
        return wxPoint(-1, -1);  // �������������Ч������һ����Ч�ĵ�
    }

private:

    // ���Ƹ��������״������������Բ�εȣ�
    void DrawComponentShape(wxDC& dc, const wxPoint& snapPoint) {
        std::ifstream file("tools/" + GetComponentFileName(tool));
        if (!file.is_open()) {
            wxLogError("Cannot open the JSON file for %s", GetComponentFileName(tool));
            return;
        }

        json componentJson;
        file >> componentJson;
        file.close();

        // ����ֱ�߲���
        dc.SetPen(wxPen(*wxBLACK, 4));
        for (const auto& line : componentJson["lines"]) {
            wxPoint start(line["start"][0].get<int>(), line["start"][1].get<int>());
            wxPoint end(line["end"][0].get<int>(), line["end"][1].get<int>());
            dc.DrawLine(snapPoint.x + start.x, snapPoint.y + start.y, snapPoint.x + end.x, snapPoint.y + end.y);
        }

        // �������߲��֣����У�
        if (componentJson.contains("splines")) {
            for (const auto& spline : componentJson["splines"]) {
                std::vector<wxPoint> points;
                for (const auto& point : spline["points"]) {
                    points.push_back(wxPoint(snapPoint.x + point[0].get<int>(), snapPoint.y + point[1].get<int>()));
                }
                dc.DrawSpline(points.size(), points.data());
            }
        }

        // ����Բ�β��֣����У�
        if (componentJson.contains("circle")) {
            auto circle = componentJson["circle"];
            int cx = circle["center"][0].get<int>();
            int cy = circle["center"][1].get<int>();
            int radius = circle["radius"].get<int>();
            dc.DrawCircle(snapPoint.x + cx, snapPoint.y + cy, radius);
        }
    }

    // ����������������
    void DrawPins(wxDC& dc, const wxPoint& snapPoint, const wxPoint& mousePos) {
        // ��������˿�
        dc.SetPen(wxPen(*wxBLACK, 4));
        for (const auto& input : inputs) {
            bool isMouseOver = IsMouseOverLine(input.first + snapPoint, input.second + snapPoint, mousePos);
            if (isMouseOver) {
                dc.SetPen(wxPen(wxColour(255, 0, 0), 7));  // ��ɫ����
            }
            else {
                dc.SetPen(wxPen(*wxBLACK, 4));  // Ĭ�Ϻ�ɫ
            }
            dc.DrawLine(snapPoint.x + input.first.x, snapPoint.y + input.first.y,
                snapPoint.x + input.second.x, snapPoint.y + input.second.y);
        }

        // ��������˿�
        for (const auto& output : outputs) {
            bool isMouseOver = IsMouseOverLine(output.first + snapPoint, output.second + snapPoint, mousePos);
            if (isMouseOver) {
                dc.SetPen(wxPen(wxColour(255, 0, 0), 7));  // ��ɫ����
            }
            else {
                dc.SetPen(wxPen(*wxBLACK, 4));  // Ĭ�Ϻ�ɫ
            }
            dc.DrawLine(snapPoint.x + output.first.x, snapPoint.y + output.first.y,
                snapPoint.x + output.second.x, snapPoint.y + output.second.y);
        }
    }

};
