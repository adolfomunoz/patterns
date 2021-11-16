#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "self-registering-factory.h"
#include "reflect.h"
#include "xml.h"

namespace pattern {
   
template<typename Self>
class SelfRegisteringReflectableBase {
public:
    virtual std::string xml_content() const = 0;
};

template<typename Self, typename... Bases>
class SelfRegisteringReflectable : public SelfRegisteringClass<Self, Bases...>, public Reflectable<Self,Bases...> {
        std::
};


}

