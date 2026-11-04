#include "data/dataType.h"

Data::Extents Data::DataType::rowMajorStrides(const Extents& dims)
{
    // The last axis moves by one element, and each previous axis by the size of everything after it.
    Extents strides = dims;
    size_t stride = 1;
    for (size_t axis = dims.size(); axis-- > 0;) {
        strides[axis] = stride;
        stride *= dims[axis];
    }
    return strides;
}

Data::DataType Data::DataType::subView(const DataType& requested, const DataType& source)
{
    if (requested.rank > source.rank) {
        throw std::invalid_argument("DataType::subView: the window has more axes than the source.");
    }

    DataType window;
    window.rank = requested.rank;
    window.dimensions = requested.dimensions;
    window.elementType = requested.elementType;
    window.elementSize = requested.elementSize;

    // The window axes are the last axes of the source, so they move in memory like the last axes of the source.
    window.strides.resize(requested.rank);
    const size_t firstAxis = source.rank - requested.rank;
    for (size_t axis = 0; axis < requested.rank; ++axis) {
        window.strides[axis] = source.strides[firstAxis + axis];
    }
    return window;
}

size_t Data::DataType::totalElements() const noexcept
{
    // A scalar (rank 0) is one element.
    size_t total = 1;
    for (size_t axis = 0; axis < this->rank; ++axis) {
        total *= this->dimensions[axis];
    }
    return total;
}

size_t Data::DataType::memorySpan() const noexcept
{
    if (this->totalElements() == 0) {
        return 0;
    }
    // The last element is at the coordinates (dimensions - 1) on every axis.
    size_t lastOffset = 0;
    for (size_t axis = 0; axis < this->rank; ++axis) {
        lastOffset += (this->dimensions[axis] - 1) * this->strides[axis];
    }
    return lastOffset + 1;
}

bool Data::DataType::isContiguous() const noexcept
{
    // Going from the last axis to the first, each axis must move by the size of all the axes after it.
    // An axis of size 1 never moves, so its stride does not matter.
    size_t expectedStride = 1;
    for (size_t axis = this->rank; axis-- > 0;) {
        if (this->dimensions[axis] != 1 && this->strides[axis] != expectedStride) {
            return false;
        }
        expectedStride *= this->dimensions[axis];
    }
    return true;
}

size_t Data::DataType::offsetOf(size_t address) const noexcept
{
    // Split the address into one coordinate per axis, starting with the last axis,
    // and add the distance in memory of each coordinate.
    size_t offset = 0;
    for (size_t axis = this->rank; axis-- > 0;) {
        offset += (address % this->dimensions[axis]) * this->strides[axis];
        address /= this->dimensions[axis];
    }
    return offset;
}

size_t Data::DataType::requestedExtent(const Data::DataType& requested, size_t rank, size_t k) const noexcept
{
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

    return requested.dimensions[k - padding];
}

bool Data::DataType::canFitIn(const DataType& requested, size_t address) const noexcept
{
    // A scalar can only contain another scalar at address zero.
    if (this->rank == 0) {
        return requested.rank == 0 && address == 0;
    }

    // A request cannot have more axes than the shape it must fit into.
    if (requested.rank > this->rank) {
        return false;
    }

    // Split the address into one coordinate per axis, starting with the last axis,
    // and check that the window stays inside the view along each of them.
    size_t remaining = address;

    for (size_t axis = this->rank; axis-- > 0;) {
        size_t dimension = this->dimensions[axis];

        // Avoid division or modulo by zero for an empty dimension.
        if (dimension == 0) {
            return false;
        }

        size_t coordinate = remaining % dimension;
        remaining /= dimension;

        size_t extent = this->requestedExtent(requested, this->rank, axis);
        if (coordinate + extent > dimension) {
            return false;
        }
    }

    // If anything remains, the address was outside the view.
    return remaining == 0;
}

size_t Data::DataType::getAddressSpace(const DataType& requested) const noexcept
{
    // A request is invalid if either element type is missing, the types differ,
    // or the requested shape cannot fit at the origin of this shape.
    if (this->elementType == nullptr ||
        requested.elementType == nullptr ||
        !sameElementType(*this->elementType, *requested.elementType) ||
        !this->canFitIn(requested, 0)) {
        return 0;
    }

    // A scalar has only one possible position.
    if (this->rank == 0) {
        return 1;
    }

    // Along an axis of size D with a window of size E, the window can start at D - E + 1 positions.
    // The positions of the axes combine by multiplication.
    size_t space = 1;
    for (size_t axis = 0; axis < this->rank; ++axis) {
        size_t extent = this->requestedExtent(requested, this->rank, axis);
        space *= this->dimensions[axis] - extent + 1;
    }
    return space;
}

size_t Data::DataType::getValidAddress(const Data::DataType& requested, const size_t address) const noexcept
{
    // For a one-dimensional shape, or a scalar request, every address is valid as it is.
    if (this->rank < 2 || requested.rank == 0) {
        return address;
    }

    // Split the address, which counts the positions where the window fits, into one coordinate per axis
    // (last axis first), and rebuild the address in the full shape from these coordinates.
    size_t remaining = address;
    size_t result = 0;
    size_t stride = 1; // Size of everything after the axis in the full shape.

    for (size_t axis = this->rank; axis-- > 0;) {
        size_t extent = this->requestedExtent(requested, this->rank, axis);

        // An oversized window cannot be mapped to a valid position.
        if (extent > this->dimensions[axis]) {
            return address;
        }

        // The window can start at this many positions along the axis (a dimension of 5 and a window of 2 give 4).
        size_t positions = this->dimensions[axis] - extent + 1;

        result += (remaining % positions) * stride;
        remaining /= positions;
        stride *= this->dimensions[axis];
    }

    return result;
}

bool Data::DataType::operator==(const DataType& other) const noexcept
{
    // Two data types are equal when their logical shapes and element types match. The layout is ignored.
    return this->rank == other.rank &&
           this->dimensions == other.dimensions &&
           sameElementType(*this->elementType, *other.elementType) &&
           this->elementSize == other.elementSize;
}

bool Data::DataType::operator!=(const DataType& other) const noexcept
{
    return !(*this == other);
}

bool Data::DataType::equalsWithLayout(const DataType& other) const noexcept
{
    return *this == other && this->strides == other.strides;
}

namespace {
    // Print a list like [2, 3].
    void printList(std::ostringstream& oss, const Data::Extents& list)
    {
        oss << "[";
        for (size_t idx = 0; idx < list.size(); ++idx) {
            if (idx > 0) {
                oss << ", ";
            }
            oss << list[idx];
        }
        oss << "]";
    }
}

std::string Data::DataType::toString() const
{
    std::ostringstream oss;

    oss << "DataType{rank=" << static_cast<int>(this->rank) << ", dimensions=";
    printList(oss, this->dimensions);
    oss << ", strides=";
    printList(oss, this->strides);
    oss << ", elementType=";

    // Print the element type's demangled name when available.
    if (this->elementType != nullptr) {
        oss << DEMANGLE_TYPEID_NAME(this->elementType->name());
    } else {
        oss << "null";
    }

    oss << ", elementSize=" << this->elementSize << "}";
    return oss.str();
}

std::string Data::DataType::summary() const
{
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
        printList(oss, this->dimensions);
        oss << "'";
    }

    return oss.str();
}
