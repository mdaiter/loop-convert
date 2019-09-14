#include <initializer_list>

constexpr auto APITypeInfo = "template<typename APIType> struct APITypeInfo { };";
constexpr auto ImplTypeInfo = "template<typename ImplType> struct ImplTypeInfo { };";
constexpr auto ADD_API_MAPPING = "#define ADD_API_MAPPING(TheAPIType, TheImplType)\n \
  template<> struct APITypeInfo<TheAPIType> { typedef TheImplType* ImplType; };\n \
  template<> struct ImplTypeInfo<TheImplType*> { typedef TheAPIType APIType; };";

/*
 * All of this will allow seamless transition between types
 * */

constexpr auto toAPI =
"template<typename T, typename APIType = typename ImplTypeInfo<T>::APIType>\n \
auto toAPI(T* t) -> APIType\n \
{\n \
    return reinterpret_cast<APIType>(t);\n \
}";

constexpr auto toImpl = 
"template<typename T, typename ImplType = typename APITypeInfo<T>::ImplType>\n \
auto toImpl(T t) -> ImplType*\n \
{\n \
    return static_cast<ImplType*>(static_cast<void*>(const_cast<typename std::remove_const<typename std::remove_pointer<T>::type>::type*>(t)));\n \
}";

constexpr auto constexprStmts = {
  APITypeInfo,
  ImplTypeInfo,
  ADD_API_MAPPING,
  toAPI,
  toImpl
};
