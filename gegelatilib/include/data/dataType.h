/**
 * @file dataType.h
 * \brief Defines DataType, a descriptor for the shape, type, and source context of data.
 *
 * DataType is a **self-contained** descriptor that encapsulates:
 * 1. **Shape**: Rank (0=scalar, 1=1D, 2=2D) and dimensions.
 * 2. **Type**: `std::type_info` and element size (for pointer arithmetic).
 * 3. **Source Context**: For sub-views, stores the original source's shape and offset.
 *
 * This design allows DataView and DataValue to contain **only a data pointer and a DataType**,
 * while still supporting:
 * - Type safety (via elementType).
 * - Bounds safety (via sourceDimensions and sourceOffset).
 * - Sub-array extraction (via getSubView).
 */

#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
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
     * \brief Descriptor of a typed view or value, including shape and source context.
     *
     * `DataType` is the metadata object carried by `DataView` and `DataValue`.
     * It answers three questions about the memory it describes:
     *
     * 1. What is the logical shape of the view? (`rank`, `dimensions`)
     * 2. What element type does the memory store? (`elementType`, `elementSize`)
     * 3. If this is a sub-view, where does it come from in the original source?
     *    (`sourceRank`, `sourceDimensions`, `sourceOffset`)
     *
     * The class is intentionally lightweight and type-erased: it stores runtime type
     * information and dimensional metadata, but it does not own the underlying buffer.
     * This allows consumers to validate operations such as typed reads, sub-view creation,
     * and row-major indexing without having to duplicate the full data layout in every class.
     *
     * ### Shape model
     * - Rank 0: scalar
     * - Rank 1: 1D array
     * - Rank 2: 2D array
     * - Rank N: N-dimensional array
     *
     * For 1D arrays, `dimensions = {size, 0, ...}`. For 2D arrays, `dimensions = {rows, cols, 0, ...}`.
     * All linear indexing is done in row-major order.
     *
     * ### Source context
     * For top-level objects, `sourceRank`, `sourceDimensions`, and `sourceOffset` match the
     * same data as the current view. For sub-views, they still point to the original buffer,
     * which allows algorithms to compute a valid window inside the source without losing the
     * original coordinate system.
     *
     * A typical example:
     * - source: `double[8]`
     * - view: `double[4]` at offset 1
     * - the view describes a window covering elements 1, 2, 3, 4 of the source.
     *
     * @note `sourceOffset` is expressed in elements, not bytes, and for 2D arrays it follows
     * row-major linearization: offset = row * width + col.
     */
    struct DataType {

        /// \brief Highest rank supported by the fixed-size stack storage of \ref forEachRun.
        static constexpr size_t MaxRank = 8;

        // --- Shape of THIS view ---
        /// \brief Number of dimensions (0=scalar, 1=1D array, 2=2D array).
        uint8_t rank = 0;

        /// \brief Extent of each dimension. For 1D: [size, 0, ...]. For 2D: [height, width, 0, ...].
        std::vector<size_t> dimensions;

        // --- Type information ---
        /// \brief Runtime type of the elements (e.g., `typeid(double)`).
        const std::type_info* elementType = nullptr;

        /// \brief Size of each element in bytes (e.g., `sizeof(double)`).
        size_t elementSize = 0;

        // --- Source context (for sub-views) ---
        /// \brief Rank of the original source (for top-level, equals `rank`).
        uint8_t sourceRank = 0;

        /// \brief Dimensions of the original source (for top-level, equals `dimensions`).
        std::vector<size_t> sourceDimensions{};

        /// \brief Linear offset (in **elements**) from the start of the source.
        /// For top-level, this is 0. For sub-views, this is the starting index in the source.
        size_t sourceOffset = 0;

        // --- Constructors ---

        /**
         * \brief Default constructor (invalid DataType).
         */
        DataType() = default;

        /**
         * \brief static constructor creating a scalar.
         *
         * \tparam T The element type of the array.
         */
        template <typename T>
        static DataType type() {
            DataType dt;
            dt.elementType = &typeid(T);
            dt.elementSize = sizeof(T);
            return dt;
        } 


        /**
         * \brief Builds a descriptor for a row-major N-dimensional array.
         *
         * \tparam T The element type of the array.
         * \param[in] dims Extent of each dimension.
         */
        template <typename T>
        static DataType type(const std::vector<size_t>& dims){

            DataType dt;
            
            dt.rank = dims.size();
            dt.dimensions = dims;
            dt.elementType = &typeid(T);
            dt.elementSize = sizeof(T);
            dt.sourceRank = dims.size();
            dt.sourceDimensions = dims;

            return dt;
        }
        /**
         * \brief Creates metadata for a sub-view while preserving source provenance.
         *
         * The source shape and offset are retained so pointer arithmetic can be reconstructed.
         * \param[in] viewShape Shape and element type of the sub-view.
         * \param[in] source Metadata of the original source.
         * \param[in] offset Linear source offset in elements.
         * \return Metadata describing the sub-view.
         */
        static DataType subView(DataType viewShape, const DataType& source, size_t offset);

        // --- Helpers ---
        /// Extent of the requested shape along axis k of a type of rank `rank` (leading axes count as 1).
        size_t requestedExtent(const Data::DataType& requested, size_t rank, size_t k) const noexcept;

        /**
         * \brief Returns the element offset, from the start of this view, of the sub-view located at address.
         *
         * The address is the row-major linear index of the sub-view origin within this view's own
         * dimensions. The result takes the strides of the underlying source into account, so it
         * differs from the address for strided sub-views.
         */
        size_t sourceOffsetOf(size_t address) const noexcept;

        /** \brief Returns true when the elements of this view are contiguous in memory. */
        bool isContiguous() const noexcept;

        /**
         * \brief Calls fn(srcOffset, blockOffset, runLength) for each contiguous run of a block.
         *
         * The block (shape given by `block`) starts at element `baseOffset` of this descriptor's
         * source, laid out with this descriptor's source strides. Runs follow the last axis of the
         * block, and `blockOffset` is the position of the run in the block's own row-major order.
         */
        template <typename F>
        void forEachRun(const DataType& block, size_t baseOffset, F&& fn) const
        {
            const size_t blockRank = block.rank;

            // A scalar always contains exactly one element.
            if (blockRank == 0) {
                fn(baseOffset, size_t{0}, size_t{1});
                return;
            }

            const size_t total = block.totalElements();

            // An empty block has nothing to copy.
            if (total == 0) {
                return;
            }

            // When the block rank exceeds the source rank, preserve the
            // direct-addressing convention used by sourceOffsetOf().
            if (blockRank > sourceRank) {
                fn(baseOffset, size_t{0}, total);
                return;
            }

            const size_t axisOffset = sourceRank - blockRank;

            // A block is contiguous when every axis except the first spans
            // the complete corresponding source dimension.
            //
            // The first axis may be shorter: consecutive rows are still
            // contiguous when all trailing dimensions are fully covered.
            bool contiguous = true;

            for (size_t k = 1; k < blockRank; ++k) {
                if (block.dimensions[k] !=
                    sourceDimensions[axisOffset + k]) {
                    contiguous = false;
                    break;
                }
            }

            // Fast path: the entire block is one contiguous memory region.
            if (contiguous) {
                fn(baseOffset, size_t{0}, total);
                return;
            }

            // The last dimension is always copied as one contiguous run.
            const size_t run = block.dimensions[blockRank - 1];

            // Number of runs along all preceding dimensions.
            const size_t runs = total / run;
            const size_t iterRank = blockRank - 1;

            // Fixed-size stack storage avoids heap allocations per invocation.
            // Strided iteration is only possible up to MaxRank axes: fail loudly
            // rather than writing past the end of the arrays below.
            if (blockRank > MaxRank) {
                throw std::length_error(
                    "DataType::forEachRun: block rank exceeds DataType::MaxRank.");
            }

            std::array<size_t, MaxRank> index{};
            std::array<size_t, MaxRank> strides;

            // Compute the source stride associated with each block dimension.
            // The last dimension has stride 1 in row-major storage.
            size_t stride = 1;

            for (size_t k = blockRank; k-- > 0;) {
                strides[k] = stride;
                stride *= sourceDimensions[axisOffset + k];
            }

            size_t srcOffset = baseOffset;

            for (size_t i = 0; i < runs; ++i) {
                // Emit one contiguous run.
                fn(srcOffset, i * run, run);

                // Advance the multidimensional index using an odometer.
                // Avoid division and modulo in the main iteration loop.
                for (size_t k = iterRank; k-- > 0;) {
                    ++index[k];

                    if (index[k] < block.dimensions[k]) {
                        // No rollover: advance one source stride.
                        srcOffset += strides[k];
                        break;
                    }

                    // Rollover: reset this coordinate and rewind its stride.
                    index[k] = 0;
                    srcOffset -=
                        (block.dimensions[k] - 1) * strides[k];

                    // Carry into the preceding dimension.
                }
            }
        }

        /** \brief Returns the number of elements in this descriptor. */
        size_t totalElements() const noexcept;

        /** \brief Returns the number of elements in the original source. */
        size_t sourceTotalElements() const noexcept;

        /**
         * \brief Checks whether a requested descriptor can fit inside this descriptor.
         *
         * This is the descriptor-level compatibility check: it verifies whether a requested
         * view can be placed at a given linear offset inside the current view's own memory
         * layout, without crossing the current view boundaries and without mixing element types.
         *
         * The check is intentionally self-contained and does not depend on any external source
         * provenance. The source metadata is preserved for bookkeeping, but the validity test is
         * performed against the current descriptor itself.
         *
         * \param[in] requested The descriptor to be placed inside this descriptor.
         * \param[in] offset Linear offset, in elements, relative to the start of this descriptor.
         * \return `true` if the requested view fits in the current descriptor, otherwise `false`.
         */
        bool canFitIn(const DataType& requested, size_t offset = 0) const noexcept;

        /**
         * \brief Return the address space reachable by the requested type inside this descriptor.
         * 
         * For example if this descriptor is an array of size 8, and the requested is an array of size 4, the address space should be 5 (It can access indices 0, 1, 2, 3 and 4, but 5, 6, and 7 would be out of bound)
         * 
         * \param[in] requested The descriptor to be placed inside this descriptor.
         */
        size_t getAddressSpace(const DataType& requested) const noexcept;

        /**
         * \brief Get the valid address for non-contiguous 2d array.
         * 
         * \param[in] requested The descriptor to be placed inside this descriptor.
         * \param[in] address Address to scale.
         */
        size_t getValidAddress(const Data::DataType& requested, const size_t address) const noexcept;

        /**
         * \brief Compares two descriptors while ignoring source context.
         *
         * \param[in] other Descriptor to compare with.
         * \return `true` when shape, element type, and element size match.
         *
         * Two DataTypes are considered identical if they describe the same view shape and
         * the same element type/size. The provenance of the underlying source is intentionally
         * not part of this comparison.
         */
        bool operator==(const DataType& other) const noexcept;

        /**
         * \brief Compares two descriptors while ignoring source context.
         *
         * \param[in] other Descriptor to compare with.
         * \return `true` when the descriptors differ.
         */
        bool operator!=(const DataType& other) const noexcept;

        /**
         * \brief Compares two descriptors including the source provenance metadata.
         *
         * This is stricter than `operator==`: two views with the same logical shape and type but
         * different source origins are not considered equal in this comparison.
         *
         * \param[in] other Descriptor to compare with.
         * \return `true` when logical metadata and source metadata match.
         */
        bool equalsWithSource(const DataType& other) const noexcept;

        /**
         * \brief Renders the descriptor as a debug-friendly string.
         *
         * The output is primarily intended for diagnostics and testing; it prints both the
         * local view metadata and the preserved source metadata.
         */
        std::string toString() const;

        /** \brief Renders the descriptor in a compact model-summary format. */
        std::string summary() const;

    };

    /**
     * \brief operator for printing
     */
    inline std::ostream& operator<<(std::ostream& os, const DataType type) {
        return os << type.toString();
    }

} // namespace Data

#endif