#include <gtest/gtest.h>

#ifdef __GNUC__
#include "data/arrayWrapper.h"

TEST(DemangleTest, demangleTypeDouble)
{
    std::string demangled;
    const std::type_info& type = typeid(double);
    ASSERT_NO_THROW(demangled = demangle(type.name()));

    ASSERT_EQ(demangled, std::string("double"));

    ASSERT_EQ(strcmp(DEMANGLE_TYPEID_NAME(type.name()), demangled.c_str()), 0);
}

TEST(DemangleTest, demangleFail)
{
    const char* toDemangle = "gegelati";
    const std::type_info& type = typeid(double);
    ASSERT_THROW(demangle(toDemangle), std::runtime_error);

    ASSERT_THROW(DEMANGLE_TYPEID_NAME(toDemangle), std::runtime_error);
}
#endif