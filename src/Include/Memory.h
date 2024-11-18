#include "pch.h"

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T, typename ... Args>
constexpr Ref<T> create_ref(Args&& ... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}
