#ifndef MAD_LIB_TEMPLATE
#define MAD_LIB_TEMPLATE

#define EXPORT __attribute__((visibility("default")))
#define IMPORT

namespace mad_lib_template
{
    template <typename T>
    EXPORT T Add(T const &first, T const &second)
    {
        return first + second;
    }
}

namespace mad_lib_template
{
    template <typename T>
    EXPORT T Mult(T const &first, T const &second)
    {
        return first * second;
    }
}

#endif // #ifndef MAD_LIB_TEMPLATE