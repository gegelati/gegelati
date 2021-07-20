#ifndef DEMANGLE_H
#define DEMANGLE_H

#include <string>

#ifdef _MSC_VER
/// Macro for getting type name in human readable format.
#define DEMANGLE_TYPEID_NAME(name) name
#elif __GNUC__
#include <cxxabi.h>
namespace Data {
    /**
     * \brief Function calling abi::__cxa_demangle to return the type name in a
     * human readable format.
     *
     * \param[in] name const char* of the type to demangle.
     * \return demangle type of name in a std::string.
     */
    std::string demangle(const char* name);
} // namespace Data
  /// Macro for getting type name in human readable format.
#define DEMANGLE_TYPEID_NAME(name) Data::demangle(name).c_str()
#else
#error Unsupported compiler (yet): Check need for name demangling of typeid.name().
#endif

#endif // DEMANGLE_H
