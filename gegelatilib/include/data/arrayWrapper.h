
#ifndef ARRAY_WRAPPER_H

#include <stdexcept>

#include "dataHandler.h"
#include "primitiveTypeArray.h"

namespace Data {

    /**
     * DataHandler for manipulating arrays of primitive data type.
     *
     * Contrary to the PrimitiveTypeArray, the ArrayWrapper does not contain its
     * data, but possesses a pointer to them.
     */
    template <class T> class ArrayWrapper : public PrimitiveTypeArray<T>
    {
      protected:
        /**
         * \brief Pointer to the array containing the data accessed through the
         * ArrayWrapper.
         */
        std::vector<T>* containerPtr;

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
            : PrimitiveTypeArray<T>(size)
        {
            this->setPointer(ptr);
        };

        /// Default destructor
        virtual ~ArrayWrapper() = default;

        /**
         * \brief Set the pointer of the ArrayWrapper.
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
    };

    template <class T>
    inline void ArrayWrapper<T>::setPointer(std::vector<T>* ptr)
    {
        // Null ptr case
        if (ptr == nullptr) {
            this->containerPtr = ptr;
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
    }
    template <class T>
    inline UntypedSharedPtr ArrayWrapper<T>::getDataAt(
        const std::type_info& type, const size_t address) const
    {
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

    template <class T> inline size_t ArrayWrapper<T>::updateHash() const
    {
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