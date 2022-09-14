#pragma once
#ifndef EUREKA_CONCAT_
#define EUREKA_CONCAT_(x,y) x##y
#define EUREKA_CONCAT(x,y) EUREKA_CONCAT_(x,y)
#endif


#ifndef EUREKA_DEFAULT_MOVEABLE_COPYABLE
#define EUREKA_DEFAULT_MOVEABLE_COPYABLE(ObjType) \
    ObjType() = default; \
    ObjType(ObjType&& that) noexcept = default; \
    ObjType& operator=(ObjType&& rhs) noexcept = default; \
    ObjType(const ObjType& that) noexcept = default; \
    ObjType& operator=(const ObjType& rhs) noexcept = default;
#endif

#ifndef EUREKA_DEFAULT_MOVEABLE
#define EUREKA_DEFAULT_MOVEABLE(ObjType) \
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

#ifndef EUREKA_NO_COPY_NO_MOVE
#define EUREKA_NO_COPY_NO_MOVE(ObjType) \
    ObjType(ObjType&& that) noexcept = delete; \
    ObjType& operator=(ObjType&& rhs) noexcept = delete; \
    ObjType(const ObjType& that) = delete; \
    ObjType& operator=(const ObjType& rhs) = delete; 
#endif