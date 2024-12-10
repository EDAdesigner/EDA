#include<wx/wx.h>
#include <wx/toolbar.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/region.h>   // 用于 `wxRegion` 类
#include <wx/artprov.h>
#include <wx/memory.h>   // 用于 `wxMemoryDC` 类
#include <vector>
#include <wx/dcbuffer.h>
#include <wx/frame.h>       // 包含框架窗口相关功能
#include <wx/treectrl.h>    // 包含树控件的相关功能
#include <fstream>
#include <nlohmann/json.hpp>
#include <wx/dc.h>

using json = nlohmann::json;


class Component {
public:
    enum class Tool { NONE, AND_GATE, OR_GATE, NOT_GATE, NAND_GATE, NOR_GATE, XOR_GATE, XNOR_GATE, }; // 定义工具类型，包括无工具、与门、或门和非门 // 定义工具类型，包括无工具、与门、或门和非门

    Tool tool;  // 组件的类型
    wxPoint position;  // 组件的位置
    std::vector<std::pair<wxPoint, wxPoint>> pins;  // 输入引脚（起点和终点）

    Component(Tool t, const wxPoint& pos)
        : tool(t), position(pos) {}

    void LoadPinsFromJson(const std::string& filename) {
        json componentJson;
        LoadComponentFromJson(filename, componentJson);

        // 解析输入端口
        if (componentJson.contains("inputs")) {
            for (const auto& input : componentJson["inputs"]) {
                wxPoint start(input["start"][0].get<int>(), input["start"][1].get<int>());
                wxPoint end(input["end"][0].get<int>(), input["end"][1].get<int>());
                pins.push_back({ start, end });
            }
        }

        // 解析输出端口
        if (componentJson.contains("outputs")) {
            for (const auto& output : componentJson["outputs"]) {
                wxPoint start(output["start"][0].get<int>(), output["start"][1].get<int>());
                wxPoint end(output["end"][0].get<int>(), output["end"][1].get<int>());
                pins.push_back({ end, start });
            }
        }
    }


    // 绘制组件本身
    void Draw(wxDC& dc, const wxPoint& mousePos) {//mousePos是对齐后的
        // 根据工具类型选择对应的 JSON 文件
        std::string filename = GetComponentFileName(tool);

        // 绘制该组件
        DrawComponentShape(dc);

        // 绘制输入输出引脚
        DrawPins(dc, mousePos);
    }

    // 更新引脚位置的方法，传入偏移量
    void UpdatePinPositions(const wxPoint& offset) {
        for (auto& pin : pins) {
            pin.first += offset;
            pin.second += offset;
        }
    }

    // 判断鼠标是否在组件区域内
    bool IsMouseOverComponent(const wxPoint& mousePos) {//mousePos是对齐hou的
        json componentJson;
        LoadComponentFromJson("tools/" + GetComponentFileName(tool), componentJson);
        
        if (abs(position.x - mousePos.x) < 25 && abs(position.y - mousePos.y) < 25) {
            return true;
        }

        return IsMouseOverPin(mousePos);
    }



    
    // 判断鼠标是否在某个引脚上
    bool IsMouseOverPin(const wxPoint& mousePos) const {
        for (const auto& pin : pins) {
            if (IsMouseOverLine(pin.first+position, pin.second+position, mousePos)) {
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
        return wxPoint(-1,-1);
    }

    // 判断鼠标是否在引脚的线段上
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

    // 绘制该组件的形状（例如线条、圆形等）
    void DrawComponentShape(wxDC& dc) {
        json componentJson;
        LoadComponentFromJson("tools/" + GetComponentFileName(tool), componentJson);

        // 绘制直线部分
        dc.SetPen(wxPen(*wxBLACK, 4));
        if (componentJson.contains("lines")) {
            for (const auto& line : componentJson["lines"]) {
                wxPoint start(line["start"][0].get<int>(), line["start"][1].get<int>());
                wxPoint end(line["end"][0].get<int>(), line["end"][1].get<int>());
                dc.DrawLine(position.x + start.x, position.y + start.y, position.x + end.x, position.y + end.y);
            }
        }

        // 绘制曲线部分（如有）
        if (componentJson.contains("splines")) {
            for (const auto& spline : componentJson["splines"]) {
                std::vector<wxPoint> points;
                for (const auto& point : spline["points"]) {
                    points.push_back(wxPoint(position.x + point[0].get<int>(), position.y + point[1].get<int>()));
                }
                dc.DrawSpline(points.size(), points.data());
            }
        }

        // 绘制圆形部分（如有）
        if (componentJson.contains("circle")) {
            auto circle = componentJson["circle"];
            int cx = circle["center"][0].get<int>();
            int cy = circle["center"][1].get<int>();
            int radius = circle["radius"].get<int>();
            dc.DrawCircle(position.x + cx, position.y + cy, radius);
        }
    }

    // 绘制输入和输出引脚
    void DrawPins(wxDC& dc, const wxPoint& mousePos) {
        // 绘制输入输出端口
        dc.SetPen(wxPen(*wxBLACK, 4));
        for (const auto& pin : pins) {
            bool isMouseOver = IsMouseOverLine(pin.first + position, pin.second + position, mousePos);
            if (isMouseOver) {
                dc.SetPen(wxPen(wxColour(255, 0, 0), 7));  // 红色高亮
            }
            else {
                dc.SetPen(wxPen(*wxBLACK, 4));  // 默认黑色
            }
            dc.DrawLine(position.x + pin.first.x, position.y + pin.first.y,
                position.x + pin.second.x, position.y + pin.second.y);
        }
    }

};
