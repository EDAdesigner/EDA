#include "MyModule.h"


bool Module::is_in_wires(string& name) {
    if (wires.find(name) != wires.end()) {
        return true;
    }
    return false;
}
