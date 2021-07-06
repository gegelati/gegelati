/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2021) :
 *
 * Cedric Leonard <cleonard@insa-rennes.fr> (2021)
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2021)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
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

#ifndef PRIMITIVE_TYPE_ARRAY_H
#define PRIMITIVE_TYPE_ARRAY_H

#include "arrayWrapper.h"
#include "dataHandler.h"

namespace Data {
    /**
     * DataHandler for manipulating arrays of a primitive data type.
     *
     * In addition to native data types T, this DataHandler can
     * also provide the following composite data type:
     * - T[n]: with $n <=$ to the size of the PrimitiveTypeArray.
     */
    template <class T> class PrimitiveTypeArray : public ArrayWrapper<T>, public virtual DataHandler
    {
      protected:
        /**
         * \brief Array storing the data of the PrimitiveTypeArray.
         */
        std::vector<T> data;

      public:
        /**
         *  \brief Constructor for the PrimitiveTypeArray class.
         *
         * \param[in] size the fixed number of elements of primitive type T
         * contained in the PrimitiveTypeArray.
         */
        PrimitiveTypeArray(size_t size = 8);

        /// Copy constructor (deep copy).
        PrimitiveTypeArray(const PrimitiveTypeArray<T>& other);

        /// Copy content from an ArrayWrapper
        PrimitiveTypeArray(const ArrayWrapper<T>& other);

        /// Default destructor.
        virtual ~PrimitiveTypeArray() = default;

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
         * \brief Assignement Operator for PrimitiveTypeArray<T>
         *
         * Copy nbElements and data from the right side argument to the
         * left side argument
         *
         * \param[in] other the left side argument, to be assigned to the right
         * side argument.
         *
         * \return the assigned PrimitiveTypeArray
         *
         * \throws std::domain_error if both arguents do not have the same size,
         * which implies that assignement cannot be successfull.
         */
        PrimitiveTypeArray<T>& operator=(const PrimitiveTypeArray<T>& other);
    };

    template <class T>
    PrimitiveTypeArray<T>::PrimitiveTypeArray(size_t size)
        : ArrayWrapper<T>(size, nullptr), data(size)
    {
        this->setPointer(&(this->data));
    }

    template <class T>
    PrimitiveTypeArray<T>::PrimitiveTypeArray(
        const PrimitiveTypeArray<T>& other)
        : Data::DataHandler{other}, ArrayWrapper<T>(other), data(other.data)
    {
        // Set the pointer to the right data
        this->setPointer(&(this->data));
    }

    template <class T>
    PrimitiveTypeArray<T>::PrimitiveTypeArray(const ArrayWrapper<T>& other)
        : Data::DataHandler{other}, ArrayWrapper<T>(other), data(this->nbElements)
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

    template <class T> inline DataHandler* PrimitiveTypeArray<T>::clone() const
    {
        // Default copy construtor does the deep copy.
        PrimitiveTypeArray<T>* result = new PrimitiveTypeArray<T>(*this);

        return result;
    }

    template <class T> void PrimitiveTypeArray<T>::resetData()
    {
        for (T& elt : this->data) {
            elt = T{0};
        }

        // Invalidate the cached hash
        this->invalidCachedHash = true;
    }

    template <class T>
    void PrimitiveTypeArray<T>::setDataAt(const std::type_info& type,
                                          const size_t address, const T& value)
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
    PrimitiveTypeArray<T>& PrimitiveTypeArray<T>::operator=(
        const PrimitiveTypeArray<T>& other)
    {
        // Guard self assignment
        if (this != &other) {
            if (this->nbElements != other.nbElements) {
                std::stringstream message;
                message << "Assigned PrimitiveTypeArray do not have the same "
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
} // namespace Data

#endif
