#ifndef DATA_VALUE_H
#define DATA_VALUE_H

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "newData/dataConcept.h"
#include "newData/dataView.h"

namespace Data {

    /**
     * \brief Owning data container with type-erased storage and full metadata.
     *
     * DataValue is the ownership-aware counterpart of DataView. It owns its buffer and
     * keeps a DataType descriptor that explains the logical shape, runtime element type,
     * and original source layout for subviews.
     *
     * The implementation deliberately separates the public API from the type-erased
     * storage model:
     * - the public class exposes factories and typed accessors,
     * - the storage logic lives in DataConcept and concrete models (scalar, 1D, 2D),
     * - non-template behavior is kept in the .cpp companion when needed.
     */
    class DataValue : public DataView {
    public:
        using Concept = detail::ValueConcept;

    private:
        /// Type-erased storage for the owned data.
        std::unique_ptr<Concept> storage;

        /// \brief Private constructor used by factory methods.
        DataValue(std::unique_ptr<Concept> storage, DataType type);

    public:
        /// \brief Deleted default constructor (DataValue must own its data).
        DataValue() = delete;

        /// \brief Deleted copy constructor (use deep-copy factory logic instead).
        DataValue(const DataValue&) = delete;

        /// \brief Move constructor.
        DataValue(DataValue&&) noexcept = default;

        /// \brief Deleted copy assignment.
        DataValue& operator=(const DataValue&) = delete;

        /// \brief Move assignment.
        DataValue& operator=(DataValue&&) noexcept = default;

        /// \brief Default destructor.
        ~DataValue() = default;

        // --- Factory Methods ---

        /**
         * \brief Creates a DataValue owning a scalar value.
         *
         * @tparam T The type of the scalar.
         * \param[in] value The scalar value to store.
         * \return A DataValue owning the scalar.
         */
        template <typename T>
        static DataValue scalar(T value) {
            return DataValue(
                std::make_unique<detail::ScalarModel<T>>(std::move(value)),
                DataType::scalar<T>()
            );
        }

        /**
         * \brief Creates a DataValue owning a 1D array.
         *
         * @tparam T The element type of the array.
         * \param[in] values A unique_ptr to the array data (takes ownership).
         * \param[in] count The number of elements in the array.
         * \return A DataValue owning the array.
         */
        template <typename T>
        static DataValue array1d(std::unique_ptr<T[]> values, size_t count) {
            return DataValue(
                std::make_unique<detail::ArrayModel<T>>(std::move(values), count),
                DataType::array1d<T>(count)
            );
        }

        /**
         * \brief Creates a DataValue owning a 1D array from **any iterable range**.
         *
         * Works with:
         * - std::vector<T>
         * - std::list<T>
         * - std::array<T, N>
         * - Raw arrays (T[])
         * - Any type with std::begin() and std::end()
         *
         * @tparam Range The container type (e.g., std::vector<T>, std::list<T>).
         * \param[in] range The iterable range to copy.
         * \return A DataValue owning a copy of the range's elements.
         */
        template <typename Range>
        static DataValue array1d(const Range& range) {
            // Extract the element type from the range's iterator
            using T = typename std::iterator_traits<
                decltype(std::begin(range))
            >::value_type;

            // Compute size and allocate
            auto begin = std::begin(range);
            auto end = std::end(range);
            size_t count = std::distance(begin, end);
            auto ptr = std::make_unique<T[]>(count);

            // Copy elements
            std::copy(begin, end, ptr.get());

            // Return as DataValue
            return DataValue(
                std::make_unique<detail::ArrayModel<T>>(std::move(ptr), count),
                DataType::array1d<T>(count)
            );
        }

        /**
         * \brief Creates a DataValue owning a 2D array (row-major).
         *
         * @tparam T The element type of the array.
         * \param[in] values A unique_ptr to the array data (takes ownership).
         * \param[in] rows The number of rows.
         * \param[in] cols The number of columns.
         * \return A DataValue owning the 2D array.
         */
        template <typename T>
        static DataValue array2d(std::unique_ptr<T[]> values, size_t rows, size_t cols) {
            return DataValue(
                std::make_unique<detail::Array2dModel<T>>(std::move(values), rows, cols),
                DataType::array2d<T>(rows, cols)
            );
        }

        /**
         * \brief Creates a DataValue owning a 2D array from a **flat 1D range** (row-major).
         *
         * @tparam Range The 1D range type (e.g., std::vector<T>, T[]).
         * \param[in] range The flat data to interpret as 2D.
         * \param[in] rows Number of rows.
         * \param[in] cols Number of columns.
         * \return A DataValue owning the 2D array.
         * @throws std::invalid_argument If `range.size() != rows * cols`.
         */
        template <typename Range>
        static DataValue array2d(const Range& range, size_t rows, size_t cols) {
            using T = typename std::iterator_traits<decltype(std::begin(range))>::value_type;
            size_t total = rows * cols;
            if (std::distance(std::begin(range), std::end(range)) != total) {
                throw std::invalid_argument(
                    "DataValue::array2d: Range size (" +
                    std::to_string(std::distance(std::begin(range), std::end(range))) +
                    ") must equal rows * cols (" + std::to_string(total) + ")."
                );
            }
            auto ptr = std::make_unique<T[]>(total);
            std::copy(std::begin(range), std::end(range), ptr.get());
            return DataValue(
                std::make_unique<detail::Array2dModel<T>>(std::move(ptr), rows, cols),
                DataType::array2d<T>(rows, cols)
            );
        }

        /**
         * \brief Creates a DataValue owning a 2D array from a **nested range** (e.g., vector<vector<T>>).
         *
         * @tparam RangeOfRanges The nested range type (e.g., std::vector<std::vector<T>>).
         * \param[in] rows_range The nested range (each inner range is a row).
         * \return A DataValue owning the 2D array.
         * @throws std::invalid_argument If inner ranges have inconsistent sizes or the outer range is empty.
         */
        template <typename RangeOfRanges>
        static DataValue array2d(const RangeOfRanges& rows_range) {
            auto outer_begin = std::begin(rows_range);
            auto outer_end = std::end(rows_range);

            if (outer_begin == outer_end) {
                throw std::invalid_argument("DataValue::array2dFromRows: Empty outer range.");
            }

            // Deduce element type from the first inner range
            using InnerIter = decltype(std::begin(*outer_begin));
            using T = typename std::iterator_traits<InnerIter>::value_type;

            // Get cols from the first inner range
            size_t cols = std::distance(std::begin(*outer_begin), std::end(*outer_begin));
            size_t rows = std::distance(outer_begin, outer_end);

            // Validate all inner ranges have the same size
            for (auto it = outer_begin; it != outer_end; ++it) {
                size_t current_cols = std::distance(std::begin(*it), std::end(*it));
                if (current_cols != cols) {
                    throw std::invalid_argument(
                        "DataValue::array2dFromRows: All inner ranges must have the same size. "
                        "Expected " + std::to_string(cols) + ", got " + std::to_string(current_cols) + "."
                    );
                }
            }

            // Allocate and copy row-by-row
            size_t total = rows * cols;
            auto ptr = std::make_unique<T[]>(total);
            size_t row_idx = 0;
            for (auto it = outer_begin; it != outer_end; ++it, ++row_idx) {
                std::copy(std::begin(*it), std::end(*it), ptr.get() + row_idx * cols);
            }

            return DataValue(
                std::make_unique<detail::Array2dModel<T>>(std::move(ptr), rows, cols),
                DataType::array2d<T>(rows, cols)
            );
        }

        template <typename T>
        static DataValue zeros(const DataType& type)
        {
            if (type.rank == 0) {
                return DataValue::scalar<T>(T{});
            }

            if (type.rank == 1) {
                size_t count = type.dimensions[0];
                auto values = std::make_unique<T[]>(count);

                return DataValue::array1d<T>(
                    std::move(values),
                    count
                );
            }

            if (type.rank == 2) {
                auto values = std::make_unique<T[]>(type.dimensions[0] * type.dimensions[1]);

                return DataValue::array2d<T>(
                    std::move(values),
                    type.dimensions[0],
                    type.dimensions[1]
                );
            }

            throw std::invalid_argument("Unsupported rank");
        }

        /**
         * \brief Creates a **deep copy** of a sub-region of this DataValue.
         *
         * Unlike `DataView::getSubView`, this method:
         * - Allocates new memory and copies the data.
         * - Returns a **new DataValue** with no source context (owns its data).
         *
         * \param[in] requested The DataType of the requested sub-region (only shape is used).
         * \param[in] address The starting address (in elements) within this DataValue.
         * \return A new DataValue owning the copied sub-region.
         * @throws std::out_of_range If the requested shape does not fit at the given address.
         * @throws std::bad_alloc If memory allocation fails.
         */
        template <typename T>
        DataValue getSubValue(DataType requested, size_t address) const {
            if (*getType().elementType != typeid(T)) {
                throw std::runtime_error("Type mismatch in getSubValue");
            }
            if (!this->canFit(requested, address)) {
                throw std::out_of_range("Requested shape does not fit at the given address");
            }

            const T* src = static_cast<const T*>(storage->data());
            size_t absOffset = getType().sourceOffset + address;
            size_t srcStride = (getType().sourceRank >= 2) ? getType().sourceDimensions[1] : 0;

            if (requested.rank == 0) {
                return DataValue::scalar<T>(src[absOffset]);
            }
            else if (requested.rank == 1) {
                size_t count = requested.dimensions[0];
                auto buffer = std::make_unique<T[]>(count);
                std::copy(src + absOffset, src + absOffset + count, buffer.get());
                return DataValue::array1d<T>(std::move(buffer), count);
            }
            else { // rank == 2
                size_t rows = requested.dimensions[0];
                size_t cols = requested.dimensions[1];
                auto buffer = std::make_unique<T[]>(rows * cols);

                size_t srcRowStart = absOffset / srcStride;
                size_t srcColStart = absOffset % srcStride;

                for (size_t i = 0; i < rows; ++i) {
                    size_t srcIndex = (srcRowStart + i) * srcStride + srcColStart;
                    std::copy(src + srcIndex, src + srcIndex + cols, buffer.get() + i * cols);
                }

                return DataValue::array2d<T>(std::move(buffer), rows, cols);
            }
        }

        void setSubValue(const DataValue& value, size_t address);

        /**
         * \brief Checks whether the owned buffer is valid.
         */
        explicit operator bool() const noexcept;

        /**
         * \brief Renders the owned value and its metadata as a debugging string.
         */
        std::string toString() const;

        /**
         * \brief return the view corresponding to the value.
         */
        Data::DataView view() const;
    };

} // namespace Data

#endif // DATA_VALUE_H