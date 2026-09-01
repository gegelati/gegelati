#include "newData/dataView.h"

namespace Data {

    bool DataView::canFit(const DataType& requested, size_t address) const noexcept {
        if (requested.elementType == nullptr || this->type.elementType == nullptr) {
            return false;
        }
        if (requested.elementType != this->type.elementType) {
            return false;
        }
        return this->type.canFitIn(requested, address);
    }

    DataView DataView::getSubView(DataType requested, size_t address) const {
        if (!this->canFit(requested, address)) {
            throw std::out_of_range(
                "DataView getSubView: Requested shape does not fit in the current view at the given address or type is wrong."
            );
        }

        size_t newOffset = this->type.sourceOffset;
        if (this->type.sourceRank >= 2 && requested.rank >= 2) {
            size_t row = (this->type.sourceOffset + address) / this->type.sourceDimensions[1];
            size_t col = (this->type.sourceOffset + address) % this->type.sourceDimensions[1];
            size_t newRowPos = (this->type.sourceOffset / this->type.sourceDimensions[1] + row);
            size_t newColPos = (this->type.sourceOffset % this->type.sourceDimensions[1] + col);
            newOffset = newRowPos * this->type.sourceDimensions[1] + newColPos;
        } else {
            newOffset = this->type.sourceOffset + address;
        }

        size_t newPtrOffset = address;
        if (this->type.sourceRank >= 2) {
            newPtrOffset += (this->type.sourceDimensions[0] - this->type.dimensions[0]) * (address / this->type.dimensions[1]);
        }
        const size_t byteOffset = newPtrOffset * this->type.elementSize;

        DataType subType = DataType::subView(
            requested,
            this->type,
            this->type.sourceOffset + newPtrOffset
        );

        return DataView(
            static_cast<const char*>(this->ptr) + byteOffset,
            std::move(subType)
        );
    }

    void DataView::canBeAccess(const std::type_info& type, size_t requiredRank) const {
        if (!this->ptr) {
            throw std::runtime_error("Null DataView access.");
        }
        if (this->type.rank < requiredRank) {
            throw std::runtime_error(
                "Cannot get required type data (rank: " + std::to_string(requiredRank) + ") from current DataView (rank: " +
                std::to_string(this->type.rank) + ")."
            );
        }
        if (*this->type.elementType != type) {
            throw std::runtime_error(
                "Type mismatch: expected " + std::string(this->type.elementType->name()) +
                ", got " + std::string(type.name()) + "."
            );
        }
    }

    DataView::operator bool() const noexcept {
        return this->ptr != nullptr;
    }

    size_t DataView::scaleLocation(const Data::DataType& required, const size_t address) const {
        return address % this->type.totalElements();
        // Update base on required for rank > 0
    }

    std::string DataView::toString() const {
        std::ostringstream oss;

        oss << "DataView{\n"
            << "\tPointer   = " << ptr << ",\n"
            << "\tDataType  = " << type.toString() << "\n"
            << "}";

        return oss.str();
    }

} // namespace Data
