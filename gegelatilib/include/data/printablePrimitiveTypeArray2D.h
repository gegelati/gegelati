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


        PrintablePrimitiveTypeArray2D(const size_t w = 2, const size_t h = 4)
            : Data::PrimitiveTypeArray2D<T>(w, h), Data::PrintableDataHandler(),
              Data::DataHandler(){};

        PrintablePrimitiveTypeArray2D(
            const PrintablePrimitiveTypeArray2D& other) = default;

        virtual Data::DataHandler* clone() const override;
    };

    template <class T>
    Data::DataHandler* PrintablePrimitiveTypeArray2D<T>::clone() const
    {

        return new PrintablePrimitiveTypeArray2D<T>(*this);
    }
} // namespace Data

#endif // GEGELATI_PRINTABLEPRIMITIVETYPEARRAY2D_H
#endif // CODE_GENERATION