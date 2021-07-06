#ifdef CODE_GENERATION
#ifndef GEGELATI_PRINTABLEDATAHANDLER_H
#define GEGELATI_PRINTABLEDATAHANDLER_H
#include "dataHandler.h"
#include <stdint.h>
#include <vector>

namespace Data {
    /**
     * \brief printableDataHandler
     *
     * //todo
     */
    class PrintableDataHandler : public virtual Data::DataHandler
    {
      public:
        PrintableDataHandler() : Data::DataHandler(){};

        /// Default destructor
        virtual ~PrintableDataHandler() = default;

        /**
         * \brief Default copy constructor.
         */
        PrintableDataHandler(const PrintableDataHandler& other) = default;

        /**
         * \brief printableDataHandler
         *
         * //todo
         */
        virtual std::vector<uint64_t> getDataIndexes(
            const std::type_info& type, const size_t address) const = 0;

        virtual std::string getTemplatedType() const = 0;
    };
} // namespace Data

#endif // GEGELATI_PRINTABLEDATAHANDLER_H
#endif // CODE_GENERATION
