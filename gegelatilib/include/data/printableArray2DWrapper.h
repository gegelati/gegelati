#ifdef CODE_GENERATION
#ifndef GEGELATI_PRINTABLEARRAY2DWRAPPER_H
#define GEGELATI_PRINTABLEARRAY2DWRAPPER_H
#include "array2DWrapper.h"
#include "printableArrayWrapper.h"
#include <vector>

namespace Data {
    template <class T>
    class PrintableArray2DWrapper : public Data::PrintableArrayWrapper<T>,
                                    public Array2DWrapper<T>
    {
      public:
        PrintableArray2DWrapper(const size_t w = 2, const size_t h = 4) : Array2DWrapper<T>(w, h);

        std::vector<uint64_t> getDataIndexes(
            const std::type_info& type, const size_t address) const override;
    };

    std::vector<uint64_t> PrintableArray2DWrapper::getDataIndexes(
        const std::type_info& type, const size_t address, uint64_t sourceIdx,
        uint64_t opIdx) const
    {
        std::vector<uint64_t> idx;

        return idx;
    }

} // namespace Data

#endif // GEGELATI_PRINTABLEARRAY2DWRAPPER_H
#endif // CODE_GENERATION