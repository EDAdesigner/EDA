// component_factory.cpp
#include "component_factory.h"
#include "and_gate.h"
#include "or_gate.h"
#include "not_gate.h"
#include "xor_gate.h" // 如果有

std::unique_ptr<IComponentDrawer> ComponentFactory::CreateComponentDrawer(Tool tool) {
    switch (tool) {
        case Tool::AND_GATE:
            return std::make_unique<AndGate>();
        case Tool::OR_GATE:
            return std::make_unique<OrGate>();
        case Tool::NOT_GATE:
            return std::make_unique<NotGate>();
        case Tool::XOR_GATE:
            return std::make_unique<XorGate>(); // 如果有
        // 可以继续添加更多元器件...
        default:
            return nullptr;
    }
}
