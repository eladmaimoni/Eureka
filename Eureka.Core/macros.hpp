
#ifndef EUREKA_CONCAT_
#define EUREKA_CONCAT_(x,y) x##y
#define EUREKA_CONCAT(x,y) EUREKA_CONCAT_(x,y)
#endif


#ifndef EUREKA_DEFAULT_MOVEABLE
#define EUREKA_DEFAULT_MOVEABLE(ObjType) \
    ~ObjType() = default; \
    ObjType() = default; \
    ObjType(ObjType&& that) = default; \
    ObjType& operator=(ObjType&& rhs) = default;
#endif