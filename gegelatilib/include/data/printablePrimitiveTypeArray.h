#ifdef CODE_GENERATION
#ifndef GEGELATI_PRINTABLEPRIMITIVETYPEARRAY_H
#define GEGELATI_PRINTABLEPRIMITIVETYPEARRAY_H

#include "primitiveTypeArray.h"
#include "printableArrayWrapper.h"

namespace Data {
    /**
     * \brief printablePrimitiveTypeArray
     * //todo
     * @tparam T
     */
    template<class T>
    class PrintablePrimitiveTypeArray : public Data::PrimitiveTypeArray<T>,
                                        public Data::PrintableDataHandler {
    public:
        PrintablePrimitiveTypeArray(size_t size = 8)
                : Data::PrimitiveTypeArray<T>(size), Data::PrintableDataHandler() {};

        /// Copy constructor (deep copy).
        PrintablePrimitiveTypeArray(const PrintablePrimitiveTypeArray<T> &other)
                : Data::PrimitiveTypeArray<T>(other) {};

        /// Copy content from an ArrayWrapper
        PrintablePrimitiveTypeArray(const PrintableArrayWrapper<T> &other)
                : Data::PrimitiveTypeArray<T>(other),
                  Data::PrintableArrayWrapper<T>(other) {};

        /// Default destructor.
        virtual ~PrintablePrimitiveTypeArray() = default;

        std::vector<uint64_t> getDataIndexes(
                const std::type_info &type, const size_t address) const override;

        std::string getTemplatedType() const override;
    };

    template <class T>
    std::vector<uint64_t> PrintablePrimitiveTypeArray<T>::getDataIndexes(
            const std::type_info& type, const size_t address) const
    {
        if (this->containerPtr == nullptr) {
            throw std::runtime_error("Null pointer access.");
        }
#ifndef NDEBUG
        // Throw exception in case of invalid arguments.
        // checkAddressAndType(type, address);
#endif

        std::vector<uint64_t> idx;
        if (type == typeid(T)) {
            idx.push_back(address);
            return idx;
        }

        // Else, we retreive all indexes of the global variable to initialize
        // the operand

        size_t operandSize = this->nbElements - this->getAddressSpace(type) + 1;
        for (size_t i = 0; i < operandSize; ++i) {
            idx.push_back(address + i);
        }
        return idx;
    }

    template <class T>
    std::string PrintablePrimitiveTypeArray<T>::getTemplatedType() const{
        return DEMANGLE_TYPEID_NAME(typeid(T).name());
    }



} // namespace Data

#endif // GEGELATI_PRINTABLEPRIMITIVETYPEARRAY_H
#endif // CODE_GENERATION