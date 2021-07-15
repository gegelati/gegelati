#ifndef GEGELATI_DEMANGLE_H
#define GEGELATI_DEMANGLE_H

#include <string>

#ifdef _MSC_VER
/// Macro for getting type name in human readable format.
#define DEMANGLE_TYPEID_NAME(name) name
#elif __GNUC__
#include <cxxabi.h>
std::string demangle(const char* name);
/// Macro for getting type name in human readable format.
#define DEMANGLE_TYPEID_NAME(name) demangle(name).c_str()
#else
#error Unsupported compiler (yet): Check need for name demangling of typeid.name().
#endif

#endif // GEGELATI_DEMANGLE_H
