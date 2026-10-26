#include "data/dataType.h"

Data::DataType Data::DataType::subView(
    DataType viewShape,
    const DataType& source,
    size_t offset
) {
    // Create a new logical view without changing the underlying source layout.
    DataType dt;

    dt.rank = viewShape.rank;
    dt.dimensions = viewShape.dimensions;
    dt.elementType = viewShape.elementType;
    dt.elementSize = viewShape.elementSize;

    // Preserve the dimensions of the original source so that the view can
    // translate its logical addresses into addresses in the original data.
    dt.sourceRank = source.sourceRank;
    dt.sourceDimensions = source.sourceDimensions;

    // Store the starting offset of this view within its source.
    dt.sourceOffset = offset;

    return dt;
}

size_t Data::DataType::totalElements() const noexcept {
    // A scalar (rank == 0) contains exactly one element.
    size_t total = 1;

    // Multiply the dimensions to obtain the total number of logical elements.
    for (size_t axis = 0; axis < this->rank; ++axis) {
        total *= this->dimensions[axis];
    }

    return total;
}

size_t Data::DataType::sourceTotalElements() const noexcept {
    // A scalar source contains exactly one element.
    size_t total = 1;

    // Compute the number of elements in the original source layout.
    for (size_t axis = 0; axis < this->sourceRank; ++axis) {
        total *= this->sourceDimensions[axis];
    }

    return total;
}

size_t Data::DataType::requestedExtent(
    const Data::DataType& requested,
    size_t rank,
    size_t k
) const noexcept {
    // A scalar request occupies one element along every axis.
    if (requested.rank == 0) {
        return 1;
    }

    // Align the requested shape with the trailing axes of the current shape.
    // For example, [3, 4] aligned with [2, 3, 4] becomes [1, 3, 4].
    size_t padding = rank - requested.rank;

    if (k < padding) {
        return 1;
    }

    // Translate the current axis into the corresponding requested axis.
    size_t requestedAxis = k - padding;

    return requested.dimensions[requestedAxis];
}

size_t Data::DataType::sourceOffsetOf(size_t address) const noexcept {
    // One-dimensional views use the same linear address as their source.
    // If the source has fewer dimensions, no multidimensional mapping is done.
    if (this->rank <= 1 || this->sourceRank < this->rank) {
        return address;
    }

    // Align the view's axes with the trailing axes of the original source.
    // For example, a rank-2 view of a rank-3 source uses source axes 1 and 2.
    size_t axisOffset = this->sourceRank - this->rank;

    // Convert the linear address into coordinates, starting from the last axis.
    size_t remaining = address;

    // Accumulate the corresponding linear address in the source layout.
    size_t result = 0;

    // The last axis has stride 1 in row-major layout.
    // Each preceding axis has a stride equal to the product of the source
    // dimensions that follow it.
    size_t stride = 1;

    // Visit the axes in reverse order to extract coordinates from the
    // least significant (last) axis toward the first axis.
    for (size_t axisCount = this->rank; axisCount > 0; --axisCount) {
        size_t axis = axisCount - 1;

        // Extract this axis's coordinate from the remaining linear address.
        size_t coordinate = remaining % this->dimensions[axis];

        // Remove the coordinate we just extracted.
        remaining /= this->dimensions[axis];

        // Convert the coordinate into its contribution to the source address.
        result += coordinate * stride;

        // Advance the stride to the next source axis.
        stride *= this->sourceDimensions[axisOffset + axis];
    }

    return result;
}

bool Data::DataType::isContiguous() const noexcept {
    // Scalars and one-dimensional views are contiguous by definition here.
    // A view whose rank exceeds the source rank also uses direct addressing.
    if (this->rank <= 1 || this->sourceRank < this->rank) {
        return true;
    }

    // Align the view's axes with the corresponding trailing source axes.
    size_t axisOffset = this->sourceRank - this->rank;

    // The last axis is contiguous if its logical dimension matches its
    // source dimension. Every preceding axis must also match for this
    // view to occupy a contiguous region in the source layout.
    for (size_t axis = 1; axis < this->rank; ++axis) {
        if (this->dimensions[axis] != this->sourceDimensions[axisOffset + axis]) {
            return false;
        }
    }

    return true;
}

bool Data::DataType::canFitIn(
    const DataType& requested,
    size_t offset
) const noexcept {
    // A scalar can only contain another scalar at offset zero.
    if (this->rank == 0) {
        return requested.rank == 0 && offset == 0;
    }

    // A request cannot have more dimensions than the shape it must fit into.
    if (requested.rank > this->rank) {
        return false;
    }

    // Interpret the linear offset as a multidimensional coordinate.
    // Coordinates are extracted from the last axis toward the first axis.
    size_t remaining = offset;

    for (size_t axisCount = this->rank; axisCount > 0; --axisCount) {
        size_t axis = axisCount - 1;
        size_t dimension = this->dimensions[axis];

        // Avoid division or modulo by zero for an empty dimension.
        if (dimension == 0) {
            return false;
        }

        // Extract the coordinate of the requested window along this axis.
        size_t coordinate = remaining % dimension;

        // Remove this axis's coordinate before processing the preceding axis.
        remaining /= dimension;

        // The requested shape is aligned to the trailing axes of this shape.
        size_t extent = this->requestedExtent(requested, this->rank, axis);

        // The requested window must not extend beyond this axis's boundary.
        if (coordinate + extent > dimension) {
            return false;
        }
    }

    // If anything remains, the offset was outside the representable range
    // of coordinates for this shape.
    return remaining == 0;
}

size_t Data::DataType::getAddressSpace(const DataType& requested) const noexcept {
    // A request is invalid if either element type is missing, the types differ,
    // or the requested shape cannot fit at the origin of this shape.
    if (this->elementType == nullptr ||
        requested.elementType == nullptr ||
        !sameElementType(*this->elementType, *requested.elementType) ||
        !this->canFitIn(requested, 0)) {
        return 0;
    }

    // A scalar has only one possible starting position.
    if (this->rank == 0) {
        return 1;
    }

    // Count the number of valid starting coordinates along every axis.
    // For a dimension of size D and a requested extent of E, there are
    // D - E + 1 possible starting positions.
    size_t space = 1;

    for (size_t axis = 0; axis < this->rank; ++axis) {
        size_t dimension = this->dimensions[axis];
        size_t extent = this->requestedExtent(requested, this->rank, axis);
        size_t positions = dimension - extent + 1;

        // Multiply the number of choices along each axis to get the total
        // number of valid starting positions in the multidimensional shape.
        space *= positions;
    }

    return space;
}

size_t Data::DataType::getValidAddress(
    const Data::DataType& requested,
    const size_t address
) const noexcept {
    // For a one-dimensional shape, or a scalar request, keep the address
    // unchanged, as in the original implementation.
    if (this->rank < 2 || requested.rank == 0) {
        return address;
    }

    // Convert the address from the compact space of valid window positions
    // into a linear address in the full logical shape.
    size_t remaining = address;

    // Accumulate the resulting linear address in row-major order.
    size_t result = 0;

    // Start with the stride of the last axis.
    size_t stride = 1;

    // Decode the compact address from the last axis toward the first axis.
    for (size_t axisCount = this->rank; axisCount > 0; --axisCount) {
        size_t axis = axisCount - 1;

        // Determine how much of the requested window occupies this axis.
        size_t extent = this->requestedExtent(requested, this->rank, axis);

        // An oversized window cannot be mapped to a valid starting position.
        if (extent > this->dimensions[axis]) {
            return address;
        }

        // The number of valid starting coordinates along this axis.
        // For example, a dimension of 5 and an extent of 2 allow 4 positions.
        size_t allowed = this->dimensions[axis] - extent + 1;

        // Decode this axis's coordinate from the compact address.
        size_t coordinate = remaining % allowed;

        // Remove the decoded coordinate before processing the preceding axis.
        remaining /= allowed;

        // Convert this coordinate to its contribution to the full-shape
        // linear address. The stride uses the full dimension, not 'allowed'.
        result += coordinate * stride;

        // Move the stride to the preceding axis in the full logical shape.
        stride *= this->dimensions[axis];
    }

    return result;
}

bool Data::DataType::operator==(const DataType& other) const noexcept {
    // Two data types are equal when their logical shapes and element types
    // match. The source layout and source offset are deliberately ignored.
    return this->rank == other.rank &&
           this->dimensions == other.dimensions &&
           sameElementType(*this->elementType, *other.elementType) &&
           this->elementSize == other.elementSize;
}

bool Data::DataType::operator!=(const DataType& other) const noexcept {
    // Inequality is the logical inverse of equality.
    return !(*this == other);
}

bool Data::DataType::equalsWithSource(const DataType& other) const noexcept {
    // Compare both the logical data type and the metadata describing where
    // the view comes from in the original source.
    return *this == other &&
           this->sourceRank == other.sourceRank &&
           this->sourceDimensions == other.sourceDimensions &&
           this->sourceOffset == other.sourceOffset;
}

std::string Data::DataType::toString() const {
    std::ostringstream oss;

    // Start printing the complete metadata representation.
    oss << "DataType{"
        << "rank=" << static_cast<int>(this->rank)
        << ", dimensions=[";

    // Print the logical dimensions in axis order.
    for (size_t axis = 0; axis < this->rank; ++axis) {
        if (axis > 0) {
            oss << ", ";
        }

        oss << this->dimensions[axis];
    }

    oss << "]"
        << ", elementType=";

    // Print the element type's demangled name when available.
    if (this->elementType != nullptr) {
        oss << DEMANGLE_TYPEID_NAME(this->elementType->name());
    } else {
        oss << "null";
    }

    oss << ", elementSize=" << this->elementSize
        << ", sourceRank=" << static_cast<int>(this->sourceRank)
        << ", sourceDimensions=[";

    // Print the dimensions of the original source layout.
    for (size_t axis = 0; axis < this->sourceRank; ++axis) {
        if (axis > 0) {
            oss << ", ";
        }

        oss << this->sourceDimensions[axis];
    }

    // Finish with the offset of this view within its source.
    oss << "]"
        << ", sourceOffset=" << this->sourceOffset
        << "}";

    return oss.str();
}

std::string Data::DataType::summary() const {
    std::ostringstream oss;

    // Begin with the element type, or use a fallback if it is unavailable.
    if (this->elementType == nullptr) {
        oss << "'unknown";
    } else {
        oss << "'" << DEMANGLE_TYPEID_NAME(this->elementType->name());
    }

    // Display a scalar differently from a multidimensional shape.
    if (this->rank == 0) {
        oss << " scalar'";
    } else {
        // Print the dimensions in order, for example [2, 3, 4].
        oss << "[" << this->dimensions[0];

        for (size_t axis = 1; axis < this->rank; ++axis) {
            oss << ", " << this->dimensions[axis];
        }

        oss << "]'";
    }

    return oss.str();
}