#include<wx/wx.h>
#include <wx/toolbar.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/region.h>   // ���� `wxRegion` ��
#include <wx/artprov.h>
#include <wx/memory.h>   // ���� `wxMemoryDC` ��
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
    enum class Tool { NONE, AND_GATE, OR_GATE, NOT_GATE, NAND_GATE, NOR_GATE, XOR_GATE, XNOR_GATE, BATTERY, BULB }; // ���幤�����ͣ������޹��ߡ����š����źͷ��� // ���幤�����ͣ������޹��ߡ����š����źͷ���

    Tool tool;  // ���������
    wxPoint position;  // �����λ��
    std::vector<std::pair<wxPoint, wxPoint>> pins;  // �������ţ������յ㣩��ע��pins�д洢��λ�þ���json�ļ��и���ԭʼλ�����ݣ����λ�ã���
    // Ҳ����˵���ڻ����е���ʵλ����ҪԪ����λ��+��λ��
//����ֵ����ʾ��ǰֵ
    bool value = false;
    bool isSelected = false;  // �������Ƿ�ѡ��
    // ���� DrawPanel Ϊ��Ԫ��
    friend class DrawPanel;

    Component(Tool t, const wxPoint& pos)
        : tool(t), position(pos) {}

    void LoadPinsFromJson(const std::string& filename) {
        json componentJson;
        LoadComponentFromJson(filename, componentJson);

        // ��������˿�
        if (componentJson.contains("inputs")) {
            for (const auto& input : componentJson["inputs"]) {
                wxPoint start(input["start"][0].get<int>(), input["start"][1].get<int>());
                wxPoint end(input["end"][0].get<int>(), input["end"][1].get<int>());
                pins.push_back({ start, end });
            }
        }

        // ��������˿�
        if (componentJson.contains("outputs")) {
            for (const auto& output : componentJson["outputs"]) {
                wxPoint start(output["start"][0].get<int>(), output["start"][1].get<int>());
                wxPoint end(output["end"][0].get<int>(), output["end"][1].get<int>());
                pins.push_back({ end, start });
            }
        }
    }



void Draw(wxDC& dc, const wxPoint& mousePos) {
    if (isSelected) {
        dc.SetPen(wxPen(*wxRED, 3));  // ������ɫ����ɫ���߿�Ϊ3��
    } else {
        dc.SetPen(wxPen(*wxBLACK, 3));  // Ĭ����ɫ����ɫ���߿�Ϊ3��
    }

    // ���������״
    DrawComponentShape(dc);

    // ��������
    DrawPins(dc, mousePos);
}


    // ��������λ�õķ���������ƫ����
    void UpdatePinPositions(const wxPoint& offset) {
        for (auto& pin : pins) {
            pin.first += offset;
            pin.second += offset;
        }
    }

    // �ж�����Ƿ������������
    bool IsMouseOverComponent(const wxPoint& mousePos) {//mousePos�Ƕ���hou��
        json componentJson;
        LoadComponentFromJson("tools/" + GetComponentFileName(tool), componentJson);

        if (abs(position.x - mousePos.x) < 25 && abs(position.y - mousePos.y) < 25) {
            return true;
        }

        return IsMouseOverPin(mousePos);
    }


    // �ж�����Ƿ���ĳ��������
    bool IsMouseOverPin(const wxPoint& mousePos) const {
        for (const auto& pin : pins) {
            if (IsMouseOverLine(pin.first + position, pin.second + position, mousePos)) {
                return true;
            }
        }
        return false;
    }

    wxPoint GetPinPosition(const wxPoint& mousePos) {
        for (const auto& pin : pins) {
            if (IsMouseOverLine(pin.first + position, pin.second + position, mousePos)) {
                return pin.first + position;
            }
        }
        return wxPoint(-1, -1);
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
        case Tool::BATTERY: return "BATTERY.json";  // ��� BATTERY �� JSON �ļ���
        case Tool::BULB: return "BULB.json";        // ��� BULB �� JSON �ļ���

        default: return "";
        }
    }

private:

    void LoadComponentFromJson(const std::string& filename, json& componentJson) const {
        std::ifstream file(filename);
        if (!file.is_open()) {
            wxLogError("Cannot open the JSON file for %s", filename);
            return;
        }
        file >> componentJson;
        file.close();
    }

    // ���Ƹ��������״������������Բ�εȣ�
    void DrawComponentShape(wxDC& dc) {
        json componentJson;
        LoadComponentFromJson("tools/" + GetComponentFileName(tool), componentJson);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);

        // ����ֱ�߲���
        dc.SetPen(wxPen(*wxBLACK, 4));
        if (componentJson.contains("lines")) {
            for (const auto& line : componentJson["lines"]) {
                wxPoint start(line["start"][0].get<int>(), line["start"][1].get<int>());
                wxPoint end(line["end"][0].get<int>(), line["end"][1].get<int>());
                dc.DrawLine(position.x + start.x, position.y + start.y, position.x + end.x, position.y + end.y);
            }
        }

        // �������߲��֣����У�
        if (componentJson.contains("splines")) {
            for (const auto& spline : componentJson["splines"]) {
                std::vector<wxPoint> points;
                for (const auto& point : spline["points"]) {
                    points.push_back(wxPoint(position.x + point[0].get<int>(), position.y + point[1].get<int>()));
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
            dc.DrawCircle(position.x + cx, position.y + cy, radius);
        }

        // ���Ƶ�Դ�˲��֣����У�
        if (componentJson.contains("battery")) {
            auto battery = componentJson["battery"];
            int cx = battery["center"][0].get<int>();
            int cy = battery["center"][1].get<int>();
            int radius = battery["radius"].get<int>();
            dc.SetBrush(wxBrush(wxColour(0, 127, 0)));
            dc.DrawCircle(position.x + cx, position.y + cy, radius);
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
        }

        // ���Ƶ��ݶ˲��֣����У�
        if (componentJson.contains("bulb")) {
            auto bulb = componentJson["bulb"];
            int cx = bulb["center"][0].get<int>();
            int cy = bulb["center"][1].get<int>();
            int radius = bulb["radius"].get<int>();
            dc.SetBrush(wxBrush(wxColour(0, 127, 0)));
            dc.DrawCircle(position.x + cx, position.y + cy, radius);
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
        }
    }

    // ����������������
    void DrawPins(wxDC& dc, const wxPoint& mousePos) {
        // ������������˿�
        dc.SetPen(wxPen(*wxBLACK, 4));
        for (const auto& pin : pins) {
            bool isMouseOver = IsMouseOverLine(pin.first + position, pin.second + position, mousePos);
            if (isMouseOver) {
                dc.SetPen(wxPen(wxColour(255, 0, 0), 7));  // ��ɫ����
            }
            else {
                dc.SetPen(wxPen(*wxBLACK, 4));  // Ĭ�Ϻ�ɫ
            }
            dc.DrawLine(position.x + pin.first.x, position.y + pin.first.y,
                position.x + pin.second.x, position.y + pin.second.y);
        }
    }
    // �������ŵľ���λ��
    void UpdatePinPositions() {
        for (auto& pin : pins) {
            pin.first = position + wxPoint(pin.first.x - position.x, pin.first.y - position.y);
            pin.second = position + wxPoint(pin.second.x - position.x, pin.second.y - position.y);
        }
    }

};

