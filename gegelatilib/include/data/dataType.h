/**
 * @file dataType.h
 * \brief Defines DataType, the descriptor of the shape, element type and memory layout of data.
 *
 * A DataType describes N-dimensional data stored in memory, in the same way as a numpy array does:
 * - the **shape** (`dimensions`): how many elements there are along each axis,
 * - the **layout** (`strides`): how many elements to skip in memory to move by one along each axis,
 * - the **element type**.
 *
 * DataView and DataValue only hold a data pointer and a DataType. Because the layout is stored in the strides,
 * a sub-view (a window inside a bigger array) is just the same pointer moved to the window origin and a
 * DataType with the window dimensions and the strides of the source: no information about the source is needed.
 *
 * DataType is a plain value (no heap allocation, trivially copyable), because views are created and copied
 * constantly while the individuals are executed.
 */

#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>
#include "data/demangle.h"

namespace Data {

    /// \brief Compares two type_info, with a pointer-equality fast path (type_info::operator== can be slow).
    inline bool sameElementType(const std::type_info& a, const std::type_info& b) noexcept {
        return &a == &b || a == b;
    }

    /**
     * \brief List of at most Capacity sizes (the extents or the strides of the axes of a DataType).
     *
     * It behaves like a small std::vector<size_t> that never allocates. It converts implicitly from
     * std::vector<size_t> and from a braced list, so `DataType::type<double>({2, 3})` works.
     */
    class Extents {
      public:
        /// Maximum number of axes of a DataType.
        static constexpr size_t Capacity = 8;

        /// Empty list.
        Extents() = default;

        /// List of the given values. \throw std::length_error if there are more than Capacity values.
        Extents(std::initializer_list<size_t> values) { this->assign(values.begin(), values.end()); }

        /// List of the given values. \throw std::length_error if there are more than Capacity values.
        Extents(const std::vector<size_t>& values) { this->assign(values.begin(), values.end()); }

        /// Number of values.
        size_t size() const noexcept { return this->count; }

        /// Return true if there is no value.
        bool empty() const noexcept { return this->count == 0; }

        /// Value at an index (not checked).
        size_t& operator[](size_t index) noexcept { return this->values[index]; }

        /// Value at an index (not checked).
        const size_t& operator[](size_t index) const noexcept { return this->values[index]; }

        /// Iterator on the first value.
        const size_t* begin() const noexcept { return this->values.data(); }

        /// Iterator after the last value.
        const size_t* end() const noexcept { return this->values.data() + this->count; }

        /// Iterator on the first value.
        size_t* begin() noexcept { return this->values.data(); }

        /// Iterator after the last value.
        size_t* end() noexcept { return this->values.data() + this->count; }

        /// Change the number of values, new values being zero. \throw std::length_error if it is more than Capacity.
        void resize(size_t newSize) {
            if (newSize > Capacity) {
                throw std::length_error("Data::Extents: at most " + std::to_string(Capacity) + " axes are supported.");
            }
            for (size_t idx = this->count; idx < newSize; ++idx) {
                this->values[idx] = 0;
            }
            this->count = newSize;
        }

        /// Copy of the values in a std::vector.
        std::vector<size_t> toVector() const { return std::vector<size_t>(this->begin(), this->end()); }

        /// Same number of values, and same values.
        bool operator==(const Extents& other) const noexcept {
            if (this->count != other.count) {
                return false;
            }
            for (size_t idx = 0; idx < this->count; ++idx) {
                if (this->values[idx] != other.values[idx]) {
                    return false;
                }
            }
            return true;
        }

        /// Different number of values, or different values.
        bool operator!=(const Extents& other) const noexcept { return !(*this == other); }

      private:
        template <typename Iterator> void assign(Iterator first, Iterator last) {
            this->count = 0;
            this->resize(static_cast<size_t>(std::distance(first, last)));
            size_t idx = 0;
            for (; first != last; ++first) {
                this->values[idx++] = *first;
            }
        }

        std::array<size_t, Capacity> values{};
        size_t count = 0;
    };

    /**
     * \brief Descriptor of a typed view or value: shape, memory layout and element type.
     *
     * ### Shape and layout
     * - `rank` is the number of axes: 0 for a scalar, 1 for an array, 2 for a matrix...
     * - `dimensions[k]` is the number of elements along axis k.
     * - `strides[k]` is the distance in memory, in elements, between two neighbours along axis k.
     *
     * The element at coordinates (c0, c1, ..., cN) is at `c0 * strides[0] + c1 * strides[1] + ... + cN * strides[N]`
     * elements from the origin of the data.
     *
     * A value stored in one block of memory is **row-major**: the last axis moves by one element, and each
     * previous axis moves by the size of everything after it. For `dimensions = {2, 3}`: `strides = {3, 1}`.
     *
     * ```
     *   memory:  a b c d e f            dimensions = {2, 3}    strides = {3, 1}
     *   matrix:  [a b c]                a = (0,0) -> 0*3 + 0*1 = 0
     *            [d e f]                e = (1,1) -> 1*3 + 1*1 = 4
     * ```
     *
     * A sub-view is a window inside bigger data. It has the **strides of the data** and its own dimensions.
     * For the window of 2x2 starting at `b`, `dimensions = {2, 2}` and `strides = {3, 1}`: the second row of the
     * window starts 3 elements after the first, and the window is not contiguous in memory (b c . e f).
     *
     * ```
     *   [a b c]      window:   [b c]     dimensions = {2, 2}   strides = {3, 1}
     *   [d e f]                [e f]     origin = b
     * ```
     *
     * ### Addresses
     * An address is a linear index among the elements of a view, in row-major order of its own dimensions
     * (`offsetOf` gives the position in memory). It is how the genotypes choose a window in an array.
     *
     * ### Element type
     * `elementType` and `elementSize` tell what the memory stores. Strides and sizes are in elements.
     */
    struct DataType {

        /// Maximum number of axes.
        static constexpr size_t MaxRank = Extents::Capacity;

        /// Number of axes: 0 for a scalar.
        uint8_t rank = 0;

        /// Number of elements along each axis (`rank` values).
        Extents dimensions;

        /// Distance in memory, in elements, between two neighbours along each axis (`rank` values).
        Extents strides;

        /// Type of the elements, null if unknown.
        const std::type_info* elementType = nullptr;

        /// Size in bytes of an element.
        size_t elementSize = 0;

        // --- Constructors ---

        /// Default descriptor: a scalar without type.
        DataType() = default;

        /**
         * \brief Descriptor of a scalar of type T.
         */
        template <typename T>
        static DataType type() {
            DataType dt;
            dt.elementType = &typeid(T);
            dt.elementSize = sizeof(T);
            return dt;
        }

        /**
         * \brief Descriptor of an array of type T stored in one block of memory (row-major).
         *
         * \param[in] dims Number of elements along each axis, at most MaxRank axes.
         * \throw std::length_error if there are more than MaxRank axes.
         */
        template <typename T>
        static DataType type(const Extents& dims) {
            DataType dt;
            dt.rank = static_cast<uint8_t>(dims.size());
            dt.dimensions = dims;
            dt.strides = rowMajorStrides(dims);
            dt.elementType = &typeid(T);
            dt.elementSize = sizeof(T);
            return dt;
        }

        /**
         * \brief Strides of an array of the given dimensions stored in one block of memory (row-major).
         *
         * For example `{2, 3, 4}` gives `{12, 4, 1}`.
         */
        static Extents rowMajorStrides(const Extents& dims);

        /**
         * \brief Descriptor of a window inside data described by `source`.
         *
         * The window has the dimensions of `requested` and the strides of the last axes of `source`
         * (a window of rank 2 in data of rank 3 is a 2D slice, with the strides of the last 2 axes).
         *
         * \param[in] requested Dimensions and element type of the window, its rank must not exceed the rank of source.
         * \param[in] source Descriptor of the data containing the window.
         * \return Descriptor of the window. The pointer of the window is the pointer of the source moved by `source.offsetOf(address)`.
         */
        static DataType subView(const DataType& requested, const DataType& source);

        // --- Size and layout ---

        /// Number of elements of the view (1 for a scalar).
        size_t totalElements() const noexcept;

        /**
         * \brief Number of elements between the first and the last element of the view in memory, both included.
         *
         * It is `totalElements()` for contiguous data, more for a window inside bigger data. It is the size
         * of the memory to copy to get a copy of the view that keeps the same layout.
         */
        size_t memorySpan() const noexcept;

        /**
         * \brief Return true if the elements of the view follow each other in memory with no gap.
         */
        bool isContiguous() const noexcept;

        /**
         * \brief Position in memory, in elements from the origin of the view, of the element at an address.
         *
         * The address is split into one coordinate per axis (last axis first), then each coordinate is
         * multiplied by the stride of its axis. For `dimensions = {2, 2}` and `strides = {3, 1}`, address 3
         * is the coordinates (1, 1), so the position is `1 * 3 + 1 * 1 = 4`.
         *
         * The address must be smaller than totalElements().
         */
        size_t offsetOf(size_t address) const noexcept;

        /**
         * \brief Call a function for each run of consecutive elements of a block of this layout.
         *
         * The block is a window of this view, with the dimensions of `block`, and it follows the strides of this
         * view along its axes (the last axes of this view). The elements of the block are visited in row-major
         * order, grouped in the longest runs that are consecutive in memory: a block that is contiguous in
         * memory is one run, otherwise there is one run per line of the block (or more when the last axes are contiguous).
         *
         * \param[in] block Dimensions of the block, its rank must not exceed the rank of this view.
         * \param[in] baseOffset Position in memory of the first element of the block.
         * \param[in] fn Called as `fn(memoryOffset, blockOffset, length)` for each run: the run covers
         *               `length` elements starting at position `memoryOffset` in memory, and they are the
         *               elements `blockOffset ... blockOffset + length - 1` of the block read as a flat array.
         */
        template <typename F>
        void forEachRun(const DataType& block, size_t baseOffset, F&& fn) const
        {
            // A scalar block is one element.
            if (block.rank == 0) {
                fn(baseOffset, size_t{0}, size_t{1});
                return;
            }
            if (block.rank > this->rank) {
                throw std::invalid_argument("DataType::forEachRun: the block has more axes than the view.");
            }

            const size_t total = block.totalElements();
            if (total == 0) {
                return;
            }

            // The axes of the block are the last axes of this view: block axis k is the axis (firstAxis + k) of this view.
            const size_t firstAxis = this->rank - block.rank;
            auto strideOf = [&](size_t blockAxis) { return this->strides[firstAxis + blockAxis]; };

            // Group the last axes of the block that follow each other in memory into one run.
            // An axis follows the previous ones when its stride is the length of the run built so far.
            // The axes that are not grouped are walked one step at a time.
            size_t runLength = 1;
            size_t nbWalkedAxes = block.rank;
            while (nbWalkedAxes > 0 && (block.dimensions[nbWalkedAxes - 1] == 1 || strideOf(nbWalkedAxes - 1) == runLength)) {
                runLength *= block.dimensions[nbWalkedAxes - 1];
                --nbWalkedAxes;
            }

            // Walk the remaining axes like an odometer: advance the last one, and carry to the previous one when it wraps.
            std::array<size_t, MaxRank> coordinate{};
            size_t memoryOffset = baseOffset;
            const size_t nbRuns = total / runLength;
            for (size_t run = 0; run < nbRuns; ++run) {
                fn(memoryOffset, run * runLength, runLength);

                for (size_t axis = nbWalkedAxes; axis-- > 0;) {
                    ++coordinate[axis];
                    memoryOffset += strideOf(axis);
                    if (coordinate[axis] < block.dimensions[axis]) {
                        break;
                    }
                    // This axis wrapped: go back to its start, and advance the previous axis.
                    memoryOffset -= coordinate[axis] * strideOf(axis);
                    coordinate[axis] = 0;
                }
            }
        }

        // --- Addressing of windows ---

        /**
         * \brief Return true if a window of the requested shape and type fits in this view at an address.
         *
         * The window must have the same element type, not more axes than this view, and stay inside the
         * view along every axis. The shape of the window is aligned with the last axes of this view.
         *
         * \param[in] requested The window.
         * \param[in] address Address of the first element of the window, in row-major order of this view.
         */
        bool canFitIn(const DataType& requested, size_t address = 0) const noexcept;

        /**
         * \brief Number of addresses at which a window of the requested type fits in this view.
         *
         * For example if this view is an array of size 8, and the window is an array of size 4, there are
         * 5 addresses (0 to 4): the others would be out of bounds. It is 0 if the element types differ.
         */
        size_t getAddressSpace(const DataType& requested) const noexcept;

        /**
         * \brief Convert an address among the positions where the window fits into an address of this view.
         *
         * In a 2D view of 5 columns, a window of 2 columns fits at 4 of the 5 columns of each line: the
         * address 4 among valid positions is the first column of the second line, which is the address 5 of the view.
         *
         * \param[in] requested The window.
         * \param[in] address Address among the valid positions, smaller than getAddressSpace(requested).
         */
        size_t getValidAddress(const DataType& requested, const size_t address) const noexcept;

        /**
         * \brief Extent along an axis of the requested window, aligned with the last axes of a view of `rank` axes.
         *
         * Axes of the view that the window does not cover count for 1. A scalar window is 1 everywhere.
         */
        size_t requestedExtent(const DataType& requested, size_t rank, size_t k) const noexcept;

        // --- Comparison ---

        /**
         * \brief Same rank, dimensions and element type. The layout (strides) is not compared.
         */
        bool operator==(const DataType& other) const noexcept;

        /// Different rank, dimensions or element type.
        bool operator!=(const DataType& other) const noexcept;

        /// Same as operator==, and same strides.
        bool equalsWithLayout(const DataType& other) const noexcept;

        // --- Printing ---

        /// Full description, for debugging and error messages.
        std::string toString() const;

        /// Short description, e.g. `'double[2, 3]'`.
        std::string summary() const;

      private:
        /// Print a list like [2, 3], used by toString.
        static void printList(std::ostream& oss, const Extents& list);
    };

    /// Print the full description of a DataType (see DataType::toString).
    inline std::ostream& operator<<(std::ostream& os, const DataType type) {
        return os << type.toString();
    }

} // namespace Data

#endif
