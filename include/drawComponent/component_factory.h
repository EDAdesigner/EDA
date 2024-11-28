// component_factory.h
#ifndef COMPONENT_FACTORY_H
#define COMPONENT_FACTORY_H

#include "tool.h"
#include "component_drawer.h"
#include <memory>

class ComponentFactory {
public:
    static std::unique_ptr<IComponentDrawer> CreateComponentDrawer(Tool tool);
};

#endif // COMPONENT_FACTORY_H
