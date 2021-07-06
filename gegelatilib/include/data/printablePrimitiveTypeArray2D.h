//
// Created by tbourgoi on 06/07/2021.
//
#ifdef CODE_GENERATION
#ifndef GEGELATI_PRINTABLEPRIMITIVETYPEARRAY2D_H
#define GEGELATI_PRINTABLEPRIMITIVETYPEARRAY2D_H

#include "primitiveTypeArray2D.h"
#include "printableDataHandler.h"

namespace Data {
    template <class T>
    class PrintablePrimitiveTypeArray2D : public Data::PrintableDataHandler,
                                          public Data::PrimitiveTypeArray2D<T>,
                                          public virtual Data::DataHandler
    {
        // todo constructor et les méthodes de retour de types
      public:
        PrintablePrimitiveTypeArray2D(
            const PrintablePrimitiveTypeArray2D& other) = default;
        /// Inherited from DataHandler
        virtual Data::DataHandler* clone() const override;

        std::vector<uint64_t> getDataIndexes(
            const std::type_info& type, const size_t address) const override;

        std::string getTemplatedType() const override;

        PrintablePrimitiveTypeArray2D(const size_t w = 2, const size_t h = 4)
            : Data::PrimitiveTypeArray2D<T>(w, h), Data::PrintableDataHandler(),
              Data::DataHandler()
        {
        }
    };

    template <class T>
    Data::DataHandler* PrintablePrimitiveTypeArray2D<T>::clone() const
    {

        return new PrintablePrimitiveTypeArray2D<T>(*this);
    }
    template <class T>
    std::vector<uint64_t> PrintablePrimitiveTypeArray2D<T>::getDataIndexes(
        const std::type_info& type, const size_t address) const
    {
        return std::vector<uint64_t>();
    }
    template <class T>
    std::string PrintablePrimitiveTypeArray2D<T>::getTemplatedType() const
    {
        return std::__cxx11::string();
    }
} // namespace Data

#endif // GEGELATI_PRINTABLEPRIMITIVETYPEARRAY2D_H
#endif // CODE_GENERATION