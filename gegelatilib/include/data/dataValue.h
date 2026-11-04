#ifndef DATA_VALUE_H
#define DATA_VALUE_H

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "data/dataConcept.h"
#include "data/dataView.h"

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
    private:
        /// \brief Type-erased storage for the owned data.
        std::unique_ptr<detail::ValueConcept> storage;

        /// \brief Private constructor used by factory methods.
        /// \param[in] storage Type-erased storage that transfers ownership to this value.
        /// \param[in] type Metadata describing the stored data.
        DataValue(std::unique_ptr<detail::ValueConcept> storage, DataType type);

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

        /**
         * \brief Creates a deep copy of this owning value.
         *
         * \return A new owning value with equivalent data and metadata.
         */
        DataValue clone() const;

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
                DataType::type<T>()
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
                std::make_unique<detail::ArrayModel<T>>(std::move(values), Extents{count}),
                DataType::type<T>({count})
            );
        }

        /**
         * \brief Creates a DataValue owning an N-dimensional array (row-major).
         *
         * \param[in] values Buffer owning the data (takes ownership), holding the product of the extents.
         * \param[in] shape Extent of each axis (1 to DataType::MAX_RANK axes).
         * \return A DataValue owning the array.
         * \throws std::invalid_argument If the rank is not supported.
         */
        template <typename T>
        static DataValue array(std::unique_ptr<T[]> values, std::initializer_list<size_t> shape) {
            return DataValue(
                std::make_unique<detail::ArrayModel<T>>(std::move(values), Extents(shape)),
                DataType::type<T>(shape)
            );
        }

        /**
         * \brief Creates a DataValue owning an N-dimensional array (row-major) from a flat range.
         *
         * \param[in] range Flat data, whose size must equal the product of the extents.
         * \param[in] shape Extent of each axis (1 to DataType::MAX_RANK axes).
         * \return A DataValue owning a copy of the data.
         * \throws std::invalid_argument If the size does not match the shape or the rank is not supported.
         */
        template <typename Range>
        static DataValue array(const Range& range, std::initializer_list<size_t> shape) {
            using T = typename std::iterator_traits<decltype(std::begin(range))>::value_type;
            DataType type = DataType::type<T>(shape);
            const size_t total = type.totalElements();
            const auto size = static_cast<size_t>(std::distance(std::begin(range), std::end(range)));
            if (size != total) {
                throw std::invalid_argument(
                    "DataValue::array failed: range size (" + std::to_string(size) +
                    ") must equal the product of the extents (" + std::to_string(total) +
                    "). Requested type: " + type.toString()
                );
            }
            auto ptr = std::make_unique<T[]>(total);
            std::copy(std::begin(range), std::end(range), ptr.get());
            return DataValue(
                std::make_unique<detail::ArrayModel<T>>(std::move(ptr), Extents(shape)),
                std::move(type)
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
                std::make_unique<detail::ArrayModel<T>>(std::move(ptr), Extents{count}),
                DataType::type<T>({count})
            );
        }

        /**
         * \brief Creates a DataValue of the given shape from a flat range (row-major).
         *
         * \param[in] range Flat data, whose size must equal type.totalElements().
         * \param[in] type Requested shape (rank 0 to DataType::MAX_RANK); only rank and dimensions are used.
         * \return A DataValue owning a copy of the data.
         * \throws std::invalid_argument If the size does not match the shape.
         */
        template <typename Range>
        static DataValue fromRange(const Range& range, const DataType& type) {
            using T = typename std::iterator_traits<decltype(std::begin(range))>::value_type;
            const size_t total = type.totalElements();
            const auto size = static_cast<size_t>(std::distance(std::begin(range), std::end(range)));
            if (size != total) {
                throw std::invalid_argument(
                    "DataValue::fromRange failed: range size (" + std::to_string(size) +
                    ") must equal the product of the extents (" + std::to_string(total) + ")."
                );
            }
            if (type.rank == 0) {
                return DataValue::scalar<T>(*std::begin(range));
            }
            auto ptr = std::make_unique<T[]>(total);
            std::copy(std::begin(range), std::end(range), ptr.get());
            return DataValue(
                std::make_unique<detail::ArrayModel<T>>(
                    std::move(ptr),
                    type.dimensions),
                DataType::type<T>(type.dimensions)
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
                std::make_unique<detail::ArrayModel<T>>(std::move(values), Extents{rows, cols}),
                DataType::type<T>({rows, cols})
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
         * \throws std::invalid_argument If `range.size() != rows * cols`.
         */
        template <typename Range>
        static DataValue array2d(const Range& range, size_t rows, size_t cols) {
            using T = typename std::iterator_traits<decltype(std::begin(range))>::value_type;
            size_t total = rows * cols;
            if (std::distance(std::begin(range), std::end(range)) != total) {
                throw std::invalid_argument(
                    "DataValue::array2d failed: range size (" +
                    std::to_string(std::distance(std::begin(range), std::end(range))) +
                    ") must equal rows * cols (" + std::to_string(total) + "). Requested type: " +
                    DataType::type<T>({rows, cols}).toString()
                );
            }
            auto ptr = std::make_unique<T[]>(total);
            std::copy(std::begin(range), std::end(range), ptr.get());
            return DataValue(
                std::make_unique<detail::ArrayModel<T>>(std::move(ptr), Extents{rows, cols}),
                DataType::type<T>({rows, cols})
            );
        }

        /**
         * \brief Creates a DataValue owning a 2D array from a **nested range** (e.g., vector<vector<T>>).
         *
         * @tparam RangeOfRanges The nested range type (e.g., std::vector<std::vector<T>>).
         * \param[in] rows_range The nested range (each inner range is a row).
         * \return A DataValue owning the 2D array.
         * \throws std::invalid_argument If inner ranges have inconsistent sizes or the outer range is empty.
         */
        template <typename RangeOfRanges>
        static DataValue array2d(const RangeOfRanges& rows_range) {
            auto outer_begin = std::begin(rows_range);
            auto outer_end = std::end(rows_range);

            if (outer_begin == outer_end) {
                throw std::invalid_argument(
                    "DataValue::array2dFromRows failed: outer range is empty; no DataValue was created."
                );
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
                        "DataValue::array2dFromRows failed: all inner ranges must have the same size. "
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
                std::make_unique<detail::ArrayModel<T>>(std::move(ptr), Extents{rows, cols}),
                DataType::type<T>({rows, cols})
            );
        }

        template <typename T>
        /**
         * \brief Creates a zero-initialized value with the requested shape and element type.
         * \tparam T Element type used for storage.
         * \param[in] dims Dimensions of the required value.
         * \return A zero-initialized owning value.
         * \throws std::invalid_argument If the requested rank is unsupported.
         */
        static DataValue zeros(const Extents& dims)
        {
            static_assert(std::is_arithmetic_v<T>,
                "ActivationFunctions::Zero value requires arithmetic values.");

            if (dims.empty()) {
                return DataValue::scalar<T>(T{});
            }

            DataType type = DataType::type<T>(dims);
            auto buffer = std::make_unique<T[]>(type.totalElements());

            return DataValue(
                std::make_unique<detail::ArrayModel<T>>(
                    std::move(buffer),
                    dims),
                std::move(type)
            );
        }
        
        template <typename T>
        /**
         * \brief Creates a zero-initialized scalar value.
         * \tparam T Element type used for storage.
         * \return A zero-initialized owning value.
         * \throws std::invalid_argument If the requested rank is unsupported.
         */
        static DataValue zeros()
        {
            static_assert(std::is_arithmetic_v<T>,
                "ActivationFunctions::Zero value requires arithmetic values.");
            return DataValue::scalar<T>(T{});
        }

        template <typename T>
        /**
         * \brief Creates a zero-initialized value with the requested type.
         * \tparam T Element type used for storage.
         * \param[in] type Dimensions of the required value.
         * \return A zero-initialized owning value.
         * \throws std::invalid_argument If the requested rank is unsupported.
         */
        static DataValue zeros(const DataType& type) {
            return zeros<T>(type.dimensions);
        }

        template <typename T>
        /**
         * \brief Creates a zero-initialized value with the requested type.
         * \tparam T Element type used for storage.
         * \param[in] dims Dimensions of the required value.
         * \return A zero-initialized owning value.
         * \throws std::invalid_argument If the requested rank is unsupported.
         */
        static DataValue zeros(std::initializer_list<size_t> dims) {
            return zeros<T>(Extents(dims));
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
         * \throws std::out_of_range If the requested shape does not fit at the given address.
         * \throws std::bad_alloc If memory allocation fails.
         */
        template <typename T>
        DataValue getSubValue(DataType requested, size_t address) const {
            if (*getType().elementType != typeid(T)) {
                throw std::runtime_error(
                    "DataValue::getSubValue failed: requested element type does not match the value.\n"
                    "Value:\n" + this->toString() + "\n"
                    "Requested type:\n" + requested.toString()
                );
            }
            if (!this->canFit(requested, address)) {
                throw std::out_of_range(
                    "DataValue::getSubValue failed at address " + std::to_string(address) + ".\n"
                    "Value:\n" + this->toString() + "\n"
                    "Requested type:\n" + requested.toString()
                );
            }

            const T* src = static_cast<const T*>(storage->data());
            const size_t base = getType().offsetOf(address);

            if (requested.rank == 0) {
                return DataValue::scalar<T>(src[base]);
            }

            auto buffer = std::make_unique<T[]>(requested.totalElements());
            getType().forEachRun(requested, base, [&](size_t srcOffset, size_t dstOffset, size_t length) {
                std::copy(src + srcOffset, src + srcOffset + length, buffer.get() + dstOffset);
            });
            return DataValue(
                std::make_unique<detail::ArrayModel<T>>(
                    std::move(buffer),
                    requested.dimensions),
                DataType::type<T>(requested.dimensions)
            );
        }

        /**
         * \brief Copies a source value into a sub-region of this value.
         * \param[in] value Source value to copy.
         * \param[in] address Starting element address in this value.
         * \throws std::runtime_error If the element types differ.
         * \throws std::out_of_range If the source shape does not fit.
         */
        void setSubValue(const DataValue& value, size_t address);

        /**
         * \brief Sets a scalar value at a specific address in this DataValue.
         * 
         * \param[in] value The scalar value to set.
         * \param[in] address Element address in this value.
         */
        template <typename T>
        void setScalarAt(T value, size_t address) {
            const std::type_info* et = this->type.elementType;
            bool fits = et != nullptr && sameElementType(*et, typeid(T));
            if (fits) {
                switch (this->type.rank) {
                case 0: fits = (address == 0); break;
                case 1: fits = (address < this->type.dimensions[0]); break;
                default: fits = (address < this->type.totalElements()); break;
                }
            }
            if (!fits) {
                throw std::out_of_range(
                    "DataValue::setScalarAt failed at address " + std::to_string(address) + ".\n"
                    "Value:\n" + this->toString() + "\n"
                    "Requested type:\n" + DataType::type<T>().toString()
                );
            }

            T* dst = static_cast<T*>(storage->data());
            size_t absOffset = address;
            dst[absOffset] = value;
        }

        /** \brief Checks whether the owned buffer and view pointer are valid. */
        explicit operator bool() const noexcept;

        /**
         * \brief Renders the owned value and its metadata as a debugging string.
         */
        virtual std::string toString(std::string prefix = "") const override;

        /**
         * \brief Renders the owned value in a short, one line, way
         */
        virtual std::string toShortString(std::string prefix = "") const;

        /**
         * \brief Renders only the stored values (no type metadata), e.g. "1.5" or "[1, 2]".
         */
        std::string toValueString() const;

        /** \brief Returns a non-owning view over the owned data. */
        Data::DataView view() const;

        /**
         * \brief Converts numeric elements and optionally repairs them with a constraint.
         *
         * Conversion is performed first. If `constraint` is supplied and does not
         * accept the converted value, its virtual conversion method repairs the value.
         *
         * \tparam S Numeric source type.
         * \tparam D Numeric destination type.
         * \param[in] source Source value to convert.
         * \return A new owning value with the same rank and dimensions.
         * \throws std::invalid_argument If the source or destination is nonnumeric,
         * the rank is unsupported, or the constraint returns no value.
         */
        template <typename S, typename D>
        static DataValue convertNumericValue(const DataValue& source) {
            static_assert(std::is_arithmetic_v<S>,
                          "DataValue::convert requires an arithmetic source type");
            static_assert(std::is_arithmetic_v<D>,
                          "DataValue::convert requires an arithmetic destination type");



            // Apply conversion
            size_t count = source.getType().totalElements();
            auto data = source.getData<S>();
            std::vector<D> values(count);
            for (size_t idx = 0; idx < count; ++idx) {
                values[idx] = static_cast<D>(data[idx]);
            }

            return DataValue::fromRange(values, source.getType());
        }

        
        /**
         * \brief Compares two DataValue.
         *
         * \param[in] other DataValue to compare with.
         * \return `true` when Storage and dataType is identical.
         */
        bool operator==(const DataValue& other) const noexcept;

        /**
         * \brief Compares two DataValue.
         *
         * \param[in] other DataValue to compare with.
         * \return `true` when the DataValue differ.
         */
        bool operator!=(const DataValue& other) const noexcept;

        /**
         * \brief Static method to compute the combined hash of multiple dataValues!
         */
        static size_t getCombinedHash(const std::vector<Data::DataValue>& values);
    };

} // namespace Data

#endif // DATA_VALUE_H