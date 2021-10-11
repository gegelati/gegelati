/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2021) :
 *
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#include <gtest/gtest.h>

#ifdef __GNUC__
#include "data/demangle.h"

TEST(DemangleTest, demangleTypeDouble)
{
    std::string demangled;
    const std::type_info& type = typeid(double);
    ASSERT_NO_THROW(demangled = Data::demangle(type.name()))
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

    ASSERT_THROW(Data::demangle(toDemangle), std::runtime_error)
        << "Error the function should be able to demangle the type "
           "\"gegelati\"";

    ASSERT_THROW(DEMANGLE_TYPEID_NAME(toDemangle), std::runtime_error)
        << "Error the macro DEMANGLE_TYPEID_NAME should be able to demangle "
           "the type \"gegelati\"";
}
#endif
