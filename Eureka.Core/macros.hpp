#pragma once
#ifndef EUREKA_CONCAT_
#define EUREKA_CONCAT_(x,y) x##y
#define EUREKA_CONCAT(x,y) EUREKA_CONCAT_(x,y)
#endif


#ifndef EUREKA_DEFAULT_MOVEABLE
#define EUREKA_DEFAULT_MOVEABLE(ObjType) \
    ~ObjType() noexcept = default; \
    ObjType() = default; \
    ObjType(ObjType&& that) noexcept = default; \
    ObjType& operator=(ObjType&& rhs) noexcept = default;
#endif

#ifndef EUREKA_DEFAULT_MOVEONLY
#define EUREKA_DEFAULT_MOVEONLY(ObjType) \
    EUREKA_DEFAULT_MOVEABLE(ObjType) \
    ObjType(const ObjType& that) = delete; \
    ObjType& operator=(const ObjType& rhs) = delete;
#endif