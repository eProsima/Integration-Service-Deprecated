#ifndef __XML_UTILS_IS__
#define __XML_UTILS_IS__

#include <tinyxml2.h>

static tinyxml2::XMLElement* _assignNextElement(tinyxml2::XMLElement *element, const std::string &name){
    if (!element->FirstChildElement(name.c_str())){
        throw 0;
    }
    return element->FirstChildElement(name.c_str());
}

static tinyxml2::XMLElement* _assignOptionalElement(tinyxml2::XMLElement *element, const std::string &name){
    return element->FirstChildElement(name.c_str());
}

#endif
