#ifndef DATA_SHAPE_H
#define DATA_SHAPE_H

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>

namespace Data {
    /**
     * Describes the logical shape of a value exchanged with an instruction.
     *
     * A shape is deliberately small and supports scalars, one-dimensional
     * arrays, and two-dimensional arrays. The dimensions are stored in row
     * order: for example, {height, width} describes a 2D value. A scalar is
     * represented by {1}, rather than by an empty shape.
     *
     * DataShape does not own or point to the data. It only describes how the
     * data should be interpreted; ownership is handled by DataValue and
     * lifetime is handled by the caller for DataView.
     */
    struct DataShape
    {
        /// Number of dimensions. Valid values are 0, 1, and 2.
        uint8_t rank = 0;

        /// Extent of each dimension. Only the first `rank` entries are meaningful.
        std::array<size_t, 2> dimensions{0, 0};

        /// Construct an unspecified shape with rank 0.
        DataShape() = default;

        /**
         * Construct a one- or two-dimensional shape.
         *
         * @param values dimension extents, for example {4} or {2, 3}.
         * @throws std::invalid_argument when no dimensions or more than two
         * dimensions are supplied.
         */
        DataShape(std::initializer_list<size_t> values)
        {
            if (values.size() == 0 || values.size() > dimensions.size()) {
                throw std::invalid_argument(
                    "DataShape must contain one or two dimensions.");
            }
            rank = static_cast<uint8_t>(values.size());
            std::copy(values.begin(), values.end(), dimensions.begin());
        }

        /**
         * Compare rank and both stored extents.
         *
         * Extents beyond the rank are also compared. Consequently, a shape
         * with the same rank and visible dimensions but different unused
         * metadata is not considered equal.
         */
        bool operator==(const DataShape& other) const
        {
            return rank == other.rank && dimensions == other.dimensions;
        }

        /** Return true when operator== would return false. */
        bool operator!=(const DataShape& other) const
        {
            return !(*this == other);
        }
    };
}

#endif