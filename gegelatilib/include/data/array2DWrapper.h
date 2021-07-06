/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020 - 2021) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020 - 2021)
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

#ifndef ARRAY_2D_WRAPPER_H
#define ARRAY_2D_WRAPPER_H

#include "data/arrayWrapper.h"
#include "data/dataHandler.h"

namespace Data {

    /**
     * \brief DataHandler for 2D arrays of primitive types.
     *
     * This specialization of the ArrayWrapper template class provides the
     * possibility to get data with the type:
     * - T: The primitive type.
     * - T[n][m]: with $n <= height$ and $m <= width$ of the
     * Array2DWrapper.
     * - T[n]: with $n <= width$ of the Array2DWrapper.
     *
     * It is important to note that only spatially coherent values will be
     * returned when arrays are requested. For example, when requesting a 1D
     * array of N pixels, the returned pixels will always be taken from a single
     * line of pixels, and will never comprise the last pixel from a line i, and
     * the first pixels from line i+1.
     * This means that the addressable space for arrays will be less than a 1D
     * ArrayWrapper with the same number of nbElements.
     *
     * Like with the ArrayWrapper, every time the data associated to the pointer
     * is modified, the invalidateCachedHash method should be called.
     */
    template <class T> class Array2DWrapper : public ArrayWrapper<T>, public virtual DataHandler
    {
      private:
        /**
         * \brief Caching mechanism for storing addressSpace for different
         * types.
         *
         * This map stores for each data type the size of the addressSpace for
         * the Array2DWrapper. The purpose of this cache is to accelerate
         * the numerous access to the addressSpace for different data types, by
         * performing it only once per data type requested by the Instructions,
         * through the ProgramExecutionEngine.
         *
         * This map is updated and used by the getAddressSpace method.
         *
         * Like the getAddressSpace method of the 2D array, this map stores all
         * type dimensions.
         */
        mutable std::map<std::type_index, std::tuple<size_t, size_t, size_t>>
            cachedAddressSpace;

      protected:
        /// Number of columns of the 2D array.
        size_t width;

        /// Number of lines of the 2D array.
        size_t height;

        /**
         * \brief Utility function for the class.
         *
         * This method implements the getAddressSpace method, with additional
         * arguments that are used to return the number of array dimensions of
         * the requested type.
         *
         * \param[in] type The requested type_info.
         * \param[out] dim1 Size of the 1st dimension (if any).
         * \param[out] dim2 Size of the 2st dimension (if any).
         */
        size_t getAddressSpace(const std::type_info& type, size_t* dim1,
                               size_t* dim2) const;

      public:
        /**
         * \brief Constructor for the 2D array.
         *
         * The size of the underlying ArrayWrapper will be $nbElements =
         * h*w$.
         *
         * \param[in] w The width of the 2D array.
         * \param[in] h The height of the 2D array.
         * \param[in] ptr the pointer managed by the Array2DWrapper.
         */
        Array2DWrapper(const size_t w = 2, const size_t h = 4,
                       std::vector<T>* ptr = nullptr);

        /// Default copy constructor
        Array2DWrapper(const Array2DWrapper& other) = default;

        /// Default destructor
        virtual ~Array2DWrapper() = default;

        /**
         * \brief Return a PrimitiveTypeArray2D<T> where all data of the
         * Array2DWrapper has been copied.
         *
         * \return a PrimitiveTypeArray2D.
         */
        virtual DataHandler* clone() const override;

        /// Inherited from DataHandler
        virtual size_t getAddressSpace(
            const std::type_info& type) const override;

        /// Inherited from DataHandler
        virtual std::vector<size_t> getAddressesAccessed(
            const std::type_info& type, const size_t address) const override;

        /// Inherited from DataHandler
        virtual UntypedSharedPtr getDataAt(const std::type_info& type,
                                           const size_t address) const override;
    };

    template <class T>
    inline Array2DWrapper<T>::Array2DWrapper(const size_t w, const size_t h,
                                             std::vector<T>* ptr)
        : ArrayWrapper<T>(w * h, ptr), height(h), width(w)
    {
    }

    // Declare the child class for use in clone
    template <class T> class PrimitiveTypeArray2D;

    template <class T> DataHandler* Array2DWrapper<T>::clone() const
    {
        // Create a constantCopy of the ArrayWrapper content.
        DataHandler* result = new PrimitiveTypeArray2D<T>(*this);

        return result;
    }

    template <typename T>
    std::vector<size_t> Array2DWrapper<T>::getAddressesAccessed(
        const std::type_info& type, const size_t address) const
    {
        // Initialize the result
        std::vector<size_t> result;

        // If the accessed address is valid fill the result.
        size_t arrayHeight;
        size_t arrayWidth;
        const size_t space =
            this->getAddressSpace(type, &arrayHeight, &arrayWidth);
        if (space > address) {
            // For the native type.
            if (type == typeid(T)) {
                result.push_back(address);
            }
            else {
                // Else, the type is the array type.
                size_t addressH = address / (this->width - arrayWidth + 1);
                size_t addressW = address % (this->width - arrayWidth + 1);
                size_t addressSrc = (addressH * this->width) + addressW;
                size_t idxDst = 0;
                for (size_t idxHeight = 0; idxHeight < arrayHeight;
                     idxHeight++) {
                    for (size_t idxWidth = 0; idxWidth < arrayWidth;
                         idxWidth++) {
                        size_t idxSrc = (idxHeight * this->width) + idxWidth;
                        result.push_back(addressSrc + idxSrc);
                    }
                }
            }
        }
        return result;
    }

    template <typename T>
    size_t Array2DWrapper<T>::getAddressSpace(const std::type_info& type) const
    {
        // Use the utility function
        return this->getAddressSpace(type, nullptr, nullptr);
    }

    template <typename T>
    size_t Array2DWrapper<T>::getAddressSpace(const std::type_info& type,
                                              size_t* dim1, size_t* dim2) const
    {
        // Has the result been cached
        auto iter = this->cachedAddressSpace.find(type);
        if (iter != this->cachedAddressSpace.end()) {
            if (dim1 != nullptr) {
                *dim1 = std::get<1>(iter->second);
            }
            if (dim2 != nullptr) {
                *dim2 = std::get<2>(iter->second);
            }
            return std::get<0>(iter->second);
        }

        size_t result = 0;
        // The parent function is not used to avoid performing 2 regex matches.
        if (type == typeid(T)) {
            this->cachedAddressSpace.emplace(
                type, std::make_tuple(this->nbElements, 0, 0));
            result = this->nbElements;
        }

        // Check if it is a 1D or 2D array
        // with a size (h and w) inferior to the container dimensions.
        if (result == 0) {
            std::string typeName = DEMANGLE_TYPEID_NAME(type.name());
            std::string regex{DEMANGLE_TYPEID_NAME(typeid(T).name())};
            regex.append("\\s*(const\\s*)?\\[([0-9]+)\\](\\[([0-9]+)\\])?");
            std::regex arrayType(regex);
            std::cmatch cm;
            if (std::regex_match(typeName.c_str(), cm, arrayType)) {
                int typeH = 0;
                int typeW = 0;
                if (cm[2].matched && !cm[4].matched) {
                    // 1D array
                    typeH = 1;
                    typeW = std::atoi(cm[2].str().c_str());
                }
                else if (cm[2].matched && cm[4].matched) {
                    // 2D array
                    typeH = std::atoi(cm[2].str().c_str());
                    typeW = std::atoi(cm[4].str().c_str());
                }

                // Make sure dimensions are valid for this array
                // Only spatially coherent data can be provided.
                // Data spanning over several lines can not be
                // provided
                if (typeH <= this->height && typeW <= this->width) {
                    result = (this->height - typeH + 1) *
                             ((this->width - typeW + 1));
                    if (dim1 != nullptr) {
                        *dim1 = typeH;
                    }
                    if (dim2 != nullptr) {
                        *dim2 = typeW;
                    }
                    this->cachedAddressSpace.emplace(
                        type, std::make_tuple(result, typeH, typeW));
                }
            }
        }

        return result;
    }

    template <typename T>
    UntypedSharedPtr Array2DWrapper<T>::getDataAt(const std::type_info& type,
                                                  const size_t address) const
    {
#ifndef NDEBUG
        // Throw exception in case of invalid arguments.
        ArrayWrapper<T>::checkAddressAndType(type, address);
#endif

        if (type == typeid(T)) {
            UntypedSharedPtr result(
                &(this->containerPtr->at(address)),
                UntypedSharedPtr::emptyDestructor<const T>());
            return result;
        }

        // Else, the only other supported type is cstyle array (1D or 2D).
        // Because 2D arrays are array with the second dimension set to 1,
        // all the following code is generic

        size_t arrayHeight = 0;
        size_t arrayWidth = 0;
        size_t addressableSpace =
            this->getAddressSpace(type, &arrayHeight, &arrayWidth);

        // Types was checked before trying to produce
        // a return value
        auto array = new T[arrayHeight * arrayWidth];

        // Copy its content
        size_t addressH = address / (this->width - arrayWidth + 1);
        size_t addressW = address % (this->width - arrayWidth + 1);
        size_t addressSrc = (addressH * this->width) + addressW;
        size_t idxDst = 0;
        for (size_t idxHeight = 0; idxHeight < arrayHeight; idxHeight++) {
            for (size_t idxWidth = 0; idxWidth < arrayWidth; idxWidth++) {
                size_t idxSrc = (idxHeight * this->width) + idxWidth;
                array[idxDst++] = this->containerPtr->at(addressSrc + idxSrc);
            }
        }

        // Create the UntypedSharedPtr
        UntypedSharedPtr result{
            std::make_shared<UntypedSharedPtr::Model<const T[]>>(array)};
        return result;
    }
} // namespace Data
#endif // !ARRAY_2D_WRAPPER_H

// Include PrimitiveTypeArray2D to ensure availability of the used clone method.
#include "data/primitiveTypeArray2D.h"
