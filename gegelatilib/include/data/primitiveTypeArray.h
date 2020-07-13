/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019)
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

#ifndef PRIMITIVE_TYPE_ARRAY
#define PRIMITIVE_TYPE_ARRAY

#include <functional>
#include <regex>
#include <sstream>
#include <typeinfo>

#include "data/hash.h"
#include "dataHandler.h"

#ifdef _MSC_VER
/// Macro for getting type name in human readable format.
#define DEMANGLE_TYPEID_NAME(name) name
#elif __GNUC__
#include <cxxabi.h>
/// Macro for getting type name in human readable format.
#define DEMANGLE_TYPEID_NAME(name)                                             \
    abi::__cxa_demangle(name, nullptr, nullptr, nullptr)
#else
#error Unsupported compiler (yet): Check need for name demangling of typeid.name().
#endif

namespace Data {
    /**
     * DataHandler for manipulating arrays of a primitive data type.
     *
     * In addition to native data types T, this DataHandler can
     * also provide the following composite data type:
     * - T[n]: with $n <=$ to the size of the PrimitiveTypeArray.
     */
    template <class T> class PrimitiveTypeArray : public DataHandler
    {
        static_assert(std::is_fundamental<T>::value,
                      "Template class PrimitiveTypeArray<T> can only be used "
                      "for primitive types.");

      protected:
        /**
         * \brief Number of elements contained in the array.
         *
         * Although this may seem redundant with the data.size() method, this
         * attribute is here to make it possible to check whether the size of
         * the data vector was modified throughout the lifetime of the
         * PrimitiveTypeArray. (Which should not be possible.)
         */
        const size_t nbElements;

        /**
         * \brief Array storing the data of the PrimitiveTypeArray.
         */
        std::vector<T> data;

        /**
         * Check whether the given type of data can be accessed at the given
         * address. Throws exception otherwise.
         *
         * \param[in] type the std::type_info of data.
         * \param[in] address the location of the data.
         * \throws std::invalid_argument if the given data type is not provided
         * by the DataHandler. \throws std::out_of_range if the given address is
         * invalid for the given data type.
         */
        void checkAddressAndType(const std::type_info& type,
                                 const size_t& address) const;

        /**
         * \brief Implementation of the updateHash method.
         */
        virtual size_t updateHash() const override;

      public:
        /**
         *  \brief Constructor for the PrimitiveTypeArray class.
         *
         * \param[in] size the fixed number of elements of primitive type T
         * contained in the PrimitiveTypeArray.
         */
        PrimitiveTypeArray(size_t size = 8);

        /// Default copy constructor.
        PrimitiveTypeArray(const PrimitiveTypeArray<T>& other) = default;

        /// Default destructor.
        virtual ~PrimitiveTypeArray() = default;

        // Inherited from DataHandler
        virtual DataHandler* clone() const override;

        // Inherited from DataHandler
        virtual size_t getAddressSpace(
            const std::type_info& type) const override;

        // Inherited from DataHandler
        virtual bool canHandle(const std::type_info& type) const override;

        // Inherited from DataHandler
        virtual size_t getLargestAddressSpace(void) const override;

        /**
         * \brief Sets all elements of the Array to 0 (or its equivalent for
         * the given template param.)
         */
        void resetData() override;

        /// Inherited from DataHandler
        virtual UntypedSharedPtr getDataAt(const std::type_info& type,
                                           const size_t address) const override;

        /// Inherited from DataHandler
        virtual std::vector<size_t> getAddressesAccessed(
            const std::type_info& type, const size_t address) const override;

        /**
         * \brief Set the data at the given address to the given value.
         *
         * This method is not (yet) part of the DataHandler class as for now,
         * the TPG engine does not need to update data of DataHandler, except
         * for the one used as registers, which are managed with a
         * PrimitiveTypeArray<double>.
         *
         * Invalidates the cache.
         *
         * \param[in] type the std::type_info of data set.
         * \param[in] address the location of the data to set.
         * \param[in] value a const reference to the PrimitiveType holding a
         *            value of the data to write in the current
         * PrimitiveTypeArray \throws std::invalid_argument if the given data
         * type is not handled by the DataHandler. \throws std::out_of_range if
         * the given address is invalid for the given data type.
         */
        void setDataAt(const std::type_info& type, const size_t address,
                       const T& value);
    };

    template <class T>
    PrimitiveTypeArray<T>::PrimitiveTypeArray(size_t size)
        : nbElements{size}, data(size)
    {
    }

    template <class T> inline DataHandler* PrimitiveTypeArray<T>::clone() const
    {
        // Default copy construtor should do the deep copy.
        DataHandler* result = new PrimitiveTypeArray<T>(*this);

        return result;
    }

    template <class T>
    bool PrimitiveTypeArray<T>::canHandle(const std::type_info& type) const
    {
        if (typeid(T) == type) {
            return true;
        }

        // Use the code in getAddressSpace to check if the type is supported.
        return (this->getAddressSpace(type) > 0);
    }

    template <class T>
    size_t PrimitiveTypeArray<T>::getAddressSpace(
        const std::type_info& type) const
    {
        if (type == typeid(T)) {
            return this->nbElements;
        }

        // If the type is an array of the primitive type
        // with a size inferior to the container.
        std::string typeName = DEMANGLE_TYPEID_NAME(type.name());
        std::string regex{DEMANGLE_TYPEID_NAME(typeid(T).name())};
        regex.append("\\s*(const\\s*)?\\[([0-9]+)\\]");
        std::regex arrayType(regex);
        std::cmatch cm;
        if (std::regex_match(typeName.c_str(), cm, arrayType)) {
            int size = std::atoi(cm[2].str().c_str());
            if (size <= this->nbElements) {
                return this->nbElements - size + 1;
            }
        }
        // Default case
        return 0;
    }

    // Inherited from DataHandler
    template <class T>
    size_t PrimitiveTypeArray<T>::getLargestAddressSpace() const
    {
        // Currently, largest addres space is for the template Type T.
        return this->nbElements;
    }

    template <class T> void PrimitiveTypeArray<T>::resetData()
    {
        for (T& elt : this->data) {
            elt = 0;
        }

        // Invalidate the cached hash
        this->invalidCachedHash = true;
    }

    template <class T>
    void PrimitiveTypeArray<T>::checkAddressAndType(const std::type_info& type,
                                                    const size_t& address) const
    {
        size_t addressSpace = this->getAddressSpace(type);
        // check type
        if (addressSpace == 0) {
            std::stringstream message;
            message << "Data type " << DEMANGLE_TYPEID_NAME(type.name())
                    << " cannot be accessed in a "
                    << DEMANGLE_TYPEID_NAME(typeid(*this).name()) << ".";
            throw std::invalid_argument(message.str());
        }

        // check location
        if (address >= addressSpace) {
            std::stringstream message;
            message << "Data type " << DEMANGLE_TYPEID_NAME(type.name())
                    << " cannot be accessed at address " << address
                    << ", address space size is " << addressSpace << ".";
            throw std::out_of_range(message.str());
        }
    }

    template <class T>
    UntypedSharedPtr PrimitiveTypeArray<T>::getDataAt(
        const std::type_info& type, const size_t address) const
    {
#ifndef NDEBUG
        // Throw exception in case of invalid arguments.
        checkAddressAndType(type, address);
#endif

        if (type == typeid(T)) {
            UntypedSharedPtr result(
                &(this->data[address]),
                UntypedSharedPtr::emptyDestructor<const T>());
            return result;
        }

        // Else, the only other supported type is cstyle array.

        // Allocate the array
        size_t arraySize = this->nbElements - this->getAddressSpace(type) + 1;
        T* array = new T[arraySize];

        // Copy its content
        for (size_t idx = 0; idx < arraySize; idx++) {
            array[idx] = this->data[address + idx];
        }

        // Create the UntypedSharedPtr
        UntypedSharedPtr result{
            std::make_shared<UntypedSharedPtr::Model<const T[]>>(array)};
        return result;
    }

    template <class T>
    std::vector<size_t> PrimitiveTypeArray<T>::getAddressesAccessed(
        const std::type_info& type, const size_t address) const
    {
        // Initialize the result
        std::vector<size_t> result;

        // If the accessed address is valid fill the result.
        const size_t space = this->getAddressSpace(type);
        if (space > address) {
            // For the native type.
            if (type == typeid(T)) {
                result.push_back(address);
            }
            else {
                // Else, the type is the array type.
                for (int i = 0; i < (this->nbElements - space + 1); i++) {
                    result.push_back(address + i);
                }
            }
        }
        return result;
    }

    template <class T>
    void PrimitiveTypeArray<T>::setDataAt(const std::type_info& type,
                                          const size_t address, const T& value)
    {
        // Throw exception in case of invalid arguments.
        checkAddressAndType(type, address);

        this->data[address] = value;

        // Invalidate the cached hash.
        this->invalidCachedHash = true;
    }

    template <class T> inline size_t PrimitiveTypeArray<T>::updateHash() const
    {
        // reset
        this->cachedHash = Data::Hash<size_t>()(this->id);

        // hasher
        Data::Hash<T> hasher;

        for (T dataElement : this->data) {
            // Rotate by 1 because otherwise, xor is comutative.
            this->cachedHash =
                (this->cachedHash >> 1) | (this->cachedHash << 63);
            this->cachedHash ^= hasher((T)dataElement);
        }

        // Validate the cached hash value
        this->invalidCachedHash = false;

        return this->cachedHash;
    }
} // namespace Data

#endif
