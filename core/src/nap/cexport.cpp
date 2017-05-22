#include "nap.h"

//using namespace nap;
//
//char* reString(const std::string& str) {
//    char* cStr = (char*) malloc(sizeof(char) * str.size());
//    strcpy(cStr, str.c_str());
//    return cStr;
//}
//
//NAP_EXPORT void freeStr(char* cStr) {
//    free(cStr);
//}
//
//// CORE
//
//NAP_EXPORT void* core_create() {
//    return new Core;
//}
//
//NAP_EXPORT void core_destroy(void* core) {
//    delete static_cast<Core*>(core);
//}
//
//NAP_EXPORT void core_initialize(void* core) {
//    static_cast<Core*>(core)->initialize();
//}
//
//NAP_EXPORT void* core_addEntity(void* core, const char* name) {
//    return &static_cast<Core*>(core)->addEntity(name);
//}
//
//NAP_EXPORT void* core_getRoot(void* core) {
//    return &static_cast<Core*>(core)->getRoot();
//}
//
//
//
//// ENTITY
//
//NAP_EXPORT void* entity_addEntity(void* entity, const char* name) {
//    return &static_cast<Entity*>(entity)->addEntity(name);
//}
//
//// OBJECT
//
//NAP_EXPORT char* object_getName(void* object) {
//    if (!object)
//        Logger::debug("Null oject");
//    Logger::debug(static_cast<Object*>(object)->getName());
//    return reString(static_cast<Object*>(object)->getName());
//}
//
//NAP_EXPORT void object_setName(void* object, const char* name) {
//    static_cast<Object*>(object)->setName(name);
//}
//
//NAP_EXPORT void* object_getParent(void* object) {
//    if (nullptr == object)
//        Logger::debug("Fail!");
//    return static_cast<Object*>(object)->getParentObject();
//}
//
//NAP_EXPORT void* object_addChild(void* object, const char* typeName, const char* name) {
//    return &static_cast<Object*>(object)->addChild(name, rtti::TypeInfo::getByName(typeName));
//}
//
//// TYPES
//
//NAP_EXPORT size_t core_getDataTypeCount(void* core) {
//    return static_cast<Core*>(core)->getModuleManager().getDataTypes().size();
//}
//
//NAP_EXPORT char* core_getDataTypeName(void* core, int idx) {
//    rtti::TypeInfo type = static_cast<Core*>(core)->getModuleManager().getDataTypes()[idx];
//    return reString(type.getName());
//}
//
//NAP_EXPORT size_t core_getOperatorTypeCount(void* core) {
//    return static_cast<Core*>(core)->getModuleManager().getOperatorTypes().size();
//}
//
//NAP_EXPORT char* core_getOperatorTypeName(void* core, int idx) {
//    rtti::TypeInfo type = static_cast<Core*>(core)->getModuleManager().getOperatorTypes()[idx];
//    return reString(type.getName());
//}
//
//NAP_EXPORT size_t core_getComponentTypeCount(void* core) {
//    return static_cast<Core*>(core)->getModuleManager().getComponentTypes().size();
//}
//
//NAP_EXPORT char* core_getComponentTypeName(void* core, int idx) {
//    rtti::TypeInfo type = static_cast<Core*>(core)->getModuleManager().getComponentTypes()[idx];
//    return reString(type.getName());
//}


