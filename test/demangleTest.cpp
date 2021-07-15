#include <gtest/gtest.h>

#ifdef __GNUC__
#include "data/demangle.h"

TEST(DemangleTest, demangleTypeDouble)
{
    std::string demangled;
    const std::type_info& type = typeid(double);
    ASSERT_NO_THROW(demangled = demangle(type.name()))
        << "Error can't demangle the primitive type double.";

    ASSERT_EQ(demangled, std::string("double"))
        << " Error the type demangle is not equal to \"double\"";

    ASSERT_EQ(strcmp(DEMANGLE_TYPEID_NAME(type.name()), demangled.c_str()), 0)
        << "Error the return type of the macro DEMANGLE_TYPEID_NAME is not "
           "equal to a const char* of the function demangle(const "
           "std::type_info&)";
}

TEST(DemangleTest, demangleFail)
{
    const char* toDemangle = "gegelati";

    ASSERT_THROW(demangle(toDemangle), std::runtime_error)
        << "Error the function should be able to demangle the type "
           "\"gegelati\"";

    ASSERT_THROW(DEMANGLE_TYPEID_NAME(toDemangle), std::runtime_error)
        << "Error the macro DEMANGLE_TYPEID_NAME should be able to demangle "
           "the type \"gegelati\"";
}
#endif