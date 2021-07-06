#ifdef CODE_GENERATION
#ifndef GEGELATI_PRINTABLEARRAYWRAPPER_H
#define GEGELATI_PRINTABLEARRAYWRAPPER_H
#include "arrayWrapper.h"
#include "printableDataHandler.h"
#include <vector>

namespace Data {
    /**
     * \brief printableArrayWrapper
     * //todo
     *
     */
    template <class T>
    class PrintableArrayWrapper : public Data::PrintableDataHandler,
                                  public virtual Data::ArrayWrapper<T>
    {
      public:
        PrintableArrayWrapper(size_t size = 8, std::vector<T>* ptr = nullptr)
            : Data::PrintableDataHandler(), Data::ArrayWrapper<T>(size, ptr){};

        /// Default destructor
        virtual ~PrintableArrayWrapper() = default;

        /// Default copy constructor.
        PrintableArrayWrapper(const PrintableArrayWrapper<T>& other) = default;

        std::vector<uint64_t> getDataIndexes(
            const std::type_info& type, const size_t address) const override;
    };
    template <class T>
    std::vector<uint64_t> PrintableArrayWrapper<T>::getDataIndexes(
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

} // namespace Data

#endif // GEGELATI_PRINTABLEARRAYWRAPPER_H
#endif // CODE_GENERATION