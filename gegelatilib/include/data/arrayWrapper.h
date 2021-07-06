/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2021) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2021)
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

#ifndef ARRAY_WRAPPER_H
#define ARRAY_WRAPPER_H

#include <functional>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>

#include "data/constant.h"
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
     * DataHandler for manipulating arrays of primitive data type.
     *
     * Contrary to the PrimitiveTypeArray, the ArrayWrapper does not contain its
     * data, but possesses a pointer to them.
     *
     * Every time the data associated to the pointer is modified, the
     * invalidateCachedHash method should be called.
     *
     * In addition to native data types T, this DataHandler can
     * also provide the following composite data type:
     * - T[n]: with $n <=$ to the size of the ArrayWrapper.
     */
    template <class T> class ArrayWrapper : public virtual DataHandler
    {
        static_assert(std::is_fundamental<T>::value ||
                          std::is_same<T, Data::Constant>(),
                      "Template class PrimitiveTypeArray<T> can only be used "
                      "for primitive types.");

      private:
        /**
         * \brief Caching mechanism for storing addressSpace for different
         * types.
         *
         * This map stores for each data type the size of the addressSpace for
         * the PrimitiveTypeArray. The purpose of this cache is to accelerate
         * the numerous access to the addressSpace for different data types, by
         * performing it only once per data type requested by the Instructions,
         * through the ProgramExecutionEngine.
         *
         * This map is updated and used by the getAddressSpace method.
         */
        mutable std::map<std::type_index, size_t> cachedAddressSpace;

      protected:
        /**
         * \brief Number of elements contained pointer vector.
         *
         * Although this may seem redundant with the containerPtr->size()
         * method, this attribute is here to make it possible to check whether
         * the size of the data vector was modified throughout the lifetime of
         * the ArrayWrapper. (Which should not be possible.)
         */
        const size_t nbElements;

        /**
         * \brief Pointer to the array containing the data accessed through the
         * ArrayWrapper.
         */
        std::vector<T>* containerPtr;

        /**
         * Check whether the given type of data can be accessed at the given
         * address. Throws exception otherwise.
         *
         * \param[in] type the std::type_info of data.
         * \param[in] address the location of the data.
         * \throws std::invalid_argument if the given data type is not provided
         * by the DataHandler.
         * \throws std::out_of_range if the given address is
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
         *  \brief Constructor for the ArrayWrapper class.
         *
         * \param[in] size the fixed number of elements of primitive type T
         * contained in the ArrayWrapper.
         * \param[in] ptr the pointer managed by the ArrayWrapper.
         *
         *  \throws std::domain_error in case the given non-null pointer points
         * to a vector that does not the expected size.
         */
        ArrayWrapper(size_t size = 8, std::vector<T>* ptr = nullptr)
            : nbElements{size}
        {
            this->setPointer(ptr);
        };

        /// Default destructor
        virtual ~ArrayWrapper() = default;

        /// Default copy constructor.
        ArrayWrapper(const ArrayWrapper<T>& other) = default;

        /**
         * \brief Return a PrimitiveTypeArray<T> where all data of the
         * ArrayWrapper has been copied.
         *
         * \return a PrimitiveTypeArray.
         */
        virtual DataHandler* clone() const override;

        /// Inherited from DataHandler
        virtual bool canHandle(const std::type_info& type) const override;

        /// Inherited from DataHandler
        virtual size_t getAddressSpace(
            const std::type_info& type) const override;

        /// Inherited from DataHandler
        virtual size_t getLargestAddressSpace(void) const override;

        /**
         * \brief Invalidate the hash of the container.
         *
         * Each time the data pointed by the ArrayWrapper is modified, this
         * method should be called to ensure that the hash value of the
         * DataHandler is properly updated.
         */
        void invalidateCachedHash();

        /// Inherited from DataHandler. Does nothing.
        void resetData() override;

        /**
         * \brief Set the pointer of the ArrayWrapper.
         *
         * This method automatically invalidates the cachedHash.
         *
         * \param[in] ptr the new pointer managed by the ArrayWrapper.
         *
         * \throws std::domain_error in case the given non-null pointer points
         * to a vector that does not have the same size as defined when
         * constructing the ArrayWrapper.
         */
        void setPointer(std::vector<T>* ptr);

        /// Inherited from DataHandler
        virtual UntypedSharedPtr getDataAt(const std::type_info& type,
                                           const size_t address) const override;

        /// Inherited from DataHandler
        virtual std::vector<size_t> getAddressesAccessed(
            const std::type_info& type, const size_t address) const override;

    };

    template <class T>
    bool ArrayWrapper<T>::canHandle(const std::type_info& type) const
    {
        if (typeid(T) == type) {
            return true;
        }

        // Use the code in getAddressSpace to check if the type is supported.
        return (this->getAddressSpace(type) > 0);
    }

    template <class T>
    void ArrayWrapper<T>::checkAddressAndType(const std::type_info& type,
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

    // Declare class for clone method
    template <class T> class PrimitiveTypeArray;

    template <class T> inline DataHandler* ArrayWrapper<T>::clone() const
    {
        // Create a constantCopy of the ArrayWrapper content.
        DataHandler* result = new PrimitiveTypeArray<T>(*this);

        return result;
    }

    template <class T>
    size_t ArrayWrapper<T>::getAddressSpace(const std::type_info& type) const
    {
        // Has the addresSpaceSize been cached
        auto iter = this->cachedAddressSpace.find(type);
        if (iter != this->cachedAddressSpace.end()) {
            return iter->second;
        }

        if (type == typeid(T)) {
            this->cachedAddressSpace.emplace(type, this->nbElements);
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
                size_t result = this->nbElements - size + 1;
                this->cachedAddressSpace.emplace(type, result);
                return result;
            }
        }
        // Default case
        return 0;
    }

    template <class T>
    std::vector<size_t> ArrayWrapper<T>::getAddressesAccessed(
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
    inline UntypedSharedPtr ArrayWrapper<T>::getDataAt(
        const std::type_info& type, const size_t address) const
    {
        if (this->containerPtr == nullptr) {
            throw std::runtime_error("Null pointer access.");
        }
#ifndef NDEBUG
        // Throw exception in case of invalid arguments.
        checkAddressAndType(type, address);
#endif

        if (type == typeid(T)) {
            UntypedSharedPtr result(
                &(this->containerPtr->at(address)),
                UntypedSharedPtr::emptyDestructor<const T>());
            return result;
        }

        // Else, the only other supported type is cstyle array.

        // Allocate the array
        size_t arraySize = this->nbElements - this->getAddressSpace(type) + 1;
        T* array = new T[arraySize];

        // Copy its content
        for (size_t idx = 0; idx < arraySize; idx++) {
            array[idx] = this->containerPtr->at(address + idx);
        }

        // Create the UntypedSharedPtr
        UntypedSharedPtr result{
            std::make_shared<UntypedSharedPtr::Model<const T[]>>(array)};
        return result;
    }

    template <class T> size_t ArrayWrapper<T>::getLargestAddressSpace() const
    {
        // Currently, largest addres space is for the template Type T.
        return this->nbElements;
    }

    template <class T> void ArrayWrapper<T>::invalidateCachedHash()
    {
        this->invalidCachedHash = true;
    }

    template <class T> void ArrayWrapper<T>::resetData()
    {
        // Does nothing;
    }

    template <class T>
    inline void ArrayWrapper<T>::setPointer(std::vector<T>* ptr)
    {
        // Null ptr case
        if (ptr == nullptr) {
            this->containerPtr = ptr;
            this->invalidCachedHash = true;
            return;
        }

        // Else
        // Check the size of the given vector
        if (ptr->size() != nbElements) {
            std::stringstream message;
            message << "Size of pointed data (" << ptr->size()
                    << ") does not correspond to the size of the ArrayWrapper ("
                    << this->nbElements << ").";
            throw std::domain_error(message.str());
        }

        // Else
        this->containerPtr = ptr;
        this->invalidCachedHash = true;
    }

    template <class T> inline size_t ArrayWrapper<T>::updateHash() const
    {
        // Null pointer case
        if (this->containerPtr == nullptr) {
            return this->cachedHash = 0;
        }

        // reset
        this->cachedHash = Data::Hash<size_t>()(this->id);

        // hasher
        Data::Hash<T> hasher;

        for (T dataElement : *(this->containerPtr)) {
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
#endif // !ARRAY_WRAPPER_H

// Include PrimitiveTypeArray to ensure availability of the used clone method.
#include "data/primitiveTypeArray.h"
