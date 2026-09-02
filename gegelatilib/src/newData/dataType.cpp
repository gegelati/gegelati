#include "newData/dataType.h"
namespace Data {

    DataType DataType::subView(DataType viewShape, const DataType& source, size_t offset) {
        DataType dt;
        dt.rank = viewShape.rank;
        dt.dimensions = viewShape.dimensions;
        dt.elementType = viewShape.elementType;
        dt.elementSize = viewShape.elementSize;
        dt.sourceRank = source.sourceRank;
        dt.sourceDimensions = source.sourceDimensions;
        dt.sourceOffset = offset;
        return dt;
    }

    size_t DataType::totalElements() const noexcept {
        return dimensions[0] * (rank >= 2 ? dimensions[1] : 1);
    }

    size_t DataType::sourceTotalElements() const noexcept {
        return sourceDimensions[0] * (sourceRank >= 2 ? sourceDimensions[1] : 1);
    }

    bool DataType::canFitIn(const DataType& requested, size_t offset) const noexcept {
        if (this->rank == 0) {
            return requested.rank == 0 && offset == 0;
        }

        if (this->rank == 1) {
            if (requested.rank == 0) {
                return offset < this->dimensions[0];
            }
            if (requested.rank == 1) {
                return offset + requested.dimensions[0] <= this->dimensions[0];
            }
            return false;
        }

        if (requested.rank == 0) {
            return offset < this->totalElements();
        }
        if (requested.rank == 1) {
            return ((offset % this->dimensions[1]) + requested.dimensions[0] <= this->dimensions[1]) &&
                   (offset + requested.dimensions[0] <= this->totalElements());
        }
        if (requested.rank == 2) {
            const size_t startRow = offset / this->dimensions[1];
            const size_t startCol = offset % this->dimensions[1];
            return (startRow + requested.dimensions[0] <= this->dimensions[0]) &&
                   (startCol + requested.dimensions[1] <= this->dimensions[1]);
        }

        return false;
    }

    bool DataType::operator==(const DataType& other) const noexcept {
        return rank == other.rank &&
               dimensions == other.dimensions &&
               elementType == other.elementType &&
               elementSize == other.elementSize;
    }

    bool DataType::operator!=(const DataType& other) const noexcept {
        return !(*this == other);
    }

    bool DataType::equalsWithSource(const DataType& other) const noexcept {
        return *this == other &&
               sourceRank == other.sourceRank &&
               sourceDimensions == other.sourceDimensions &&
               sourceOffset == other.sourceOffset;
    }

    std::string DataType::toString() const {
        std::ostringstream oss;

        oss << "DataType{"
            << "rank=" << static_cast<int>(this->rank)
            << ", dimensions=[";

        for (size_t i = 0; i < this->rank; ++i) {
            if (i > 0) {
                oss << ", ";
            }
            oss << this->dimensions[i];
        }

        oss << "]"
            << ", elementType=";

        if (this->elementType) {
            oss << this->elementType->name();
        } else {
            oss << "null";
        }

        oss << ", elementSize=" << this->elementSize
            << ", sourceRank=" << static_cast<int>(this->sourceRank)
            << ", sourceDimensions=[";

        for (size_t i = 0; i < this->sourceRank; ++i) {
            if (i > 0) {
                oss << ", ";
            }
            oss << this->sourceDimensions[i];
        }

        oss << "]"
            << ", sourceOffset=" << this->sourceOffset
            << "}";

        return oss.str();
    }

} // namespace Data
