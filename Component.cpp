#include<wx/wx.h>
#include <wx/toolbar.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/artprov.h>
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
    std::vector<std::pair<wxPoint, wxPoint>> inputs;  // 输入引脚（起点和终点）
    std::vector<std::pair<wxPoint, wxPoint>> outputs; // 输出引脚（起点和终点）

    Component(Tool t, const wxPoint& pos)
        : tool(t), position(pos) {}

    // 从 JSON 文件加载组件的引脚信息
    void LoadPinsFromJson(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            wxLogError("Cannot open the JSON file for %s", filename);
            return;
        }

        json componentJson;
        file >> componentJson;
        file.close();

        // 解析输入端口
        if (componentJson.contains("inputs")) {
            for (const auto& input : componentJson["inputs"]) {
                wxPoint start(input["start"][0].get<int>(), input["start"][1].get<int>());
                wxPoint end(input["end"][0].get<int>(), input["end"][1].get<int>());
                inputs.push_back({ start, end });
            }
        }

        // 解析输出端口
        if (componentJson.contains("outputs")) {
            for (const auto& output : componentJson["outputs"]) {
                wxPoint start(output["start"][0].get<int>(), output["start"][1].get<int>());
                wxPoint end(output["end"][0].get<int>(), output["end"][1].get<int>());
                outputs.push_back({ start, end });
            }
        }
    }

    // 绘制组件本身
    void Draw(wxDC& dc, const wxPoint& snapPoint, const wxPoint& mousePos) {
        // 根据工具类型选择对应的 JSON 文件
        std::string filename = GetComponentFileName(tool);

        // 绘制该组件
        DrawComponentShape(dc, snapPoint);

        // 绘制输入输出引脚
        DrawPins(dc, snapPoint, mousePos);
    }

    // 判断鼠标是否在组件区域内
    bool IsMouseOverComponent(const wxPoint& mousePos) const {
        int width = 50;  // 假设组件宽度为50像素
        int height = 50; // 假设组件高度为50像素
        return mousePos.x >= position.x && mousePos.x <= position.x + width &&
            mousePos.y >= position.y && mousePos.y <= position.y + height;
    }
    
    // 判断鼠标是否在某个引脚上
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

    // 获取指定引脚的起始位置
    wxPoint GetPinPosition(size_t pinIndex, bool isInput) const {
        if (isInput && pinIndex < inputs.size()) {
            return inputs[pinIndex].first + position;  // 输入引脚的起始位置
        }
        if (!isInput && pinIndex < outputs.size()) {
            return outputs[pinIndex].first + position;  // 输出引脚的起始位置
        }
        return wxPoint(-1, -1);  // 如果引脚索引无效，返回一个无效的点
    }

private:

    // 绘制该组件的形状（例如线条、圆形等）
    void DrawComponentShape(wxDC& dc, const wxPoint& snapPoint) {
        std::ifstream file("tools/" + GetComponentFileName(tool));
        if (!file.is_open()) {
            wxLogError("Cannot open the JSON file for %s", GetComponentFileName(tool));
            return;
        }

        json componentJson;
        file >> componentJson;
        file.close();

        // 绘制直线部分
        dc.SetPen(wxPen(*wxBLACK, 4));
        for (const auto& line : componentJson["lines"]) {
            wxPoint start(line["start"][0].get<int>(), line["start"][1].get<int>());
            wxPoint end(line["end"][0].get<int>(), line["end"][1].get<int>());
            dc.DrawLine(snapPoint.x + start.x, snapPoint.y + start.y, snapPoint.x + end.x, snapPoint.y + end.y);
        }

        // 绘制曲线部分（如有）
        if (componentJson.contains("splines")) {
            for (const auto& spline : componentJson["splines"]) {
                std::vector<wxPoint> points;
                for (const auto& point : spline["points"]) {
                    points.push_back(wxPoint(snapPoint.x + point[0].get<int>(), snapPoint.y + point[1].get<int>()));
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
            dc.DrawCircle(snapPoint.x + cx, snapPoint.y + cy, radius);
        }
    }

    // 绘制输入和输出引脚
    void DrawPins(wxDC& dc, const wxPoint& snapPoint, const wxPoint& mousePos) {
        // 绘制输入端口
        dc.SetPen(wxPen(*wxBLACK, 4));
        for (const auto& input : inputs) {
            bool isMouseOver = IsMouseOverLine(input.first + snapPoint, input.second + snapPoint, mousePos);
            if (isMouseOver) {
                dc.SetPen(wxPen(wxColour(255, 0, 0), 7));  // 红色高亮
            }
            else {
                dc.SetPen(wxPen(*wxBLACK, 4));  // 默认黑色
            }
            dc.DrawLine(snapPoint.x + input.first.x, snapPoint.y + input.first.y,
                snapPoint.x + input.second.x, snapPoint.y + input.second.y);
        }

        // 绘制输出端口
        for (const auto& output : outputs) {
            bool isMouseOver = IsMouseOverLine(output.first + snapPoint, output.second + snapPoint, mousePos);
            if (isMouseOver) {
                dc.SetPen(wxPen(wxColour(255, 0, 0), 7));  // 红色高亮
            }
            else {
                dc.SetPen(wxPen(*wxBLACK, 4));  // 默认黑色
            }
            dc.DrawLine(snapPoint.x + output.first.x, snapPoint.y + output.first.y,
                snapPoint.x + output.second.x, snapPoint.y + output.second.y);
        }
    }

};
