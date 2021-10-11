/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020 - 2021) :
 *
 * Cedric Leonard <cleonard@insa-rennes.fr> (2021)
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020 - 2021)
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

#ifndef PRIMITIVE_TYPE_ARRAY_2D_H
#define PRIMITIVE_TYPE_ARRAY_2D_H

#include <tuple>

#include "data/array2DWrapper.h"
#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"

namespace Data {

    /**
     * \brief DataHandler for 2D arrays of primitive types.
     *
     * This specialization of the Array2DWrapper template class provides the
     * possibility to get data with the type:
     * - T: The primitive type.
     * - T[n][m]: with $n <= height$ and $m <= width$ of the
     * PrimitiveTypeArray2D.
     * - T[n]: with $n <= width$ of the PrimitiveTypeArray2D.
     *
     * It is important to note that only spatially coherent values will be
     * returned when arrays are requested. For example, when requesting a 1D
     * array of N pixels, the returned pixels will always be taken from a single
     * line of pixels, and will never comprise the last pixel from a line i, and
     * the first pixels from line i+1.
     * This means that the addressable space for arrays will be less than a 1D
     * PrimitiveDataArray with the same number of nbElements.
     *
     */
    template <typename T> class PrimitiveTypeArray2D : public Array2DWrapper<T>
    {
      protected:
        /**
         * \brief Array storing the data of the PrimitiveTypeArray2D.
         */
        std::vector<T> data;

      public:
        /**
         * \brief Constructor for the 2D array.
         *
         * The size of the underlying PrimitiveTypeArray will be $nbElements =
         * h*w$.
         *
         * \param[in] w The width of the 2D array.
         * \param[in] h The height of the 2D array.
         */
        PrimitiveTypeArray2D(const size_t w = 2, const size_t h = 4);

        /// Copy constructor.
        PrimitiveTypeArray2D(const PrimitiveTypeArray2D<T>& other);

        /// Copy content from an Array2DWrapper
        PrimitiveTypeArray2D(const Array2DWrapper<T>& other);

        /// Inherited from DataHandler
        virtual DataHandler* clone() const override;

        /**
         * \brief Sets all elements of the Array to 0 (or its equivalent for
         * the given template param.)
         */
        void resetData() override;

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
        /**
         * \brief Assignement Operator for PrimitiveTypeArray2D<T>
         *
         * Copy nbElements and data from the right side argument to the
         * left side argument
         *
         * \param[in] other the left side argument, to be assigned to the right
         * side argument.
         *
         * \return the assigned PrimitiveTypeArray2D
         *
         * \throws std::domain_error if both arguents do not have the same size,
         * which implies that assignement cannot be successfull.
         */
        PrimitiveTypeArray2D<T>& operator=(
            const PrimitiveTypeArray2D<T>& other);
    };

    template <typename T>
    inline PrimitiveTypeArray2D<T>::PrimitiveTypeArray2D(const size_t w,
                                                         const size_t h)
        : Array2DWrapper<T>(w, h, nullptr), data(h * w)
    {
        // Set the pointer to the right data
        this->setPointer(&(this->data));
    }

    template <typename T>
    inline PrimitiveTypeArray2D<T>::PrimitiveTypeArray2D(
        const PrimitiveTypeArray2D<T>& other)
        : Array2DWrapper<T>(other), data(other.data)
    {
        // Set the pointer to the right data
        this->setPointer(&(this->data));
    }

    template <class T>
    PrimitiveTypeArray2D<T>::PrimitiveTypeArray2D(
        const Array2DWrapper<T>& other)
        : Array2DWrapper<T>(other), data(this->nbElements)
    {
        if (this->containerPtr != NULL) {
            // Copy the data from the given ArrayWrapper
            for (size_t i = 0; i < this->nbElements; i++) {
                // exploit the fact that the container pointer still points to
                // data from other.
                this->data[i] = this->containerPtr->at(i);
            }
        }
        else {
            this->resetData();
        }

        // Set the pointer to the right data
        this->setPointer(&(this->data));
    }

    template <typename T>
    inline DataHandler* PrimitiveTypeArray2D<T>::clone() const
    {
        // Copy construtor should do the deep copy.
        DataHandler* result = new PrimitiveTypeArray2D<T>(*this);

        return result;
    }

    template <class T> void PrimitiveTypeArray2D<T>::resetData()
    {
        for (T& elt : this->data) {
            elt = T{0};
        }

        // Invalidate the cached hash
        this->invalidCachedHash = true;
    }

    template <class T>
    void PrimitiveTypeArray2D<T>::setDataAt(const std::type_info& type,
                                            const size_t address,
                                            const T& value)
    {
#ifndef NDEBUG
        // Throw exception in case of invalid arguments.
        this->checkAddressAndType(type, address);
#endif

        this->data.at(address) = value;

        // Invalidate the cached hash.
        this->invalidCachedHash = true;
    }

    template <class T>
    PrimitiveTypeArray2D<T>& PrimitiveTypeArray2D<T>::operator=(
        const PrimitiveTypeArray2D<T>& other)
    {
        // Guard self assignment
        if (this != &other) {
            if (this->nbElements != other.nbElements) {
                std::stringstream message;
                message << "Assigned PrimitiveTypeArray2D do not have the same "
                           "size : "
                        << this->nbElements << " / " << other.nbElements << ".";
                throw std::domain_error(message.str());
            }

            // Copy Data from right arg to this
            for (auto i = 0; i < this->nbElements; i++) {
                this->data.at(i) = other.data.at(i);
            }
        }
        return *this;
    }
}; // namespace Data
#endif
