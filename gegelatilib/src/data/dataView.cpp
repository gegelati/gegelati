#include "data/dataView.h"


bool Data::DataView::canFit(const DataType& requested, size_t address) const noexcept {
    if (requested.elementType == nullptr || this->type.elementType == nullptr) {
        return false;
    }
    if (*requested.elementType != *this->type.elementType) {
        return false;
    }
    return this->type.canFitIn(requested, address);
}

Data::DataView Data::DataView::getSubView(DataType requested, size_t address) const {
    if (!this->canFit(requested, address)) {
        throw std::out_of_range(
            "Data::DataView::getSubView failed at address " + std::to_string(address) + ".\n"
            "Current view:\n" + this->toString() + "\n"
            "Requested type:\n" + requested.toString()
        );
    }

    // Preserve the original source coordinate when the view is itself a sub-view.
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

    // Account for skipped columns when a 2D sub-view starts on a later row.
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

void Data::DataView::canBeAccess(const std::type_info& type) const {
    if (!this->ptr) {
        throw std::runtime_error(
            "DataView access failed: the view has no data pointer.\n" + this->toString()
        );
    }
    if (*this->type.elementType != type) {
        throw std::runtime_error(
            "DataView access failed: requested element type " + std::string(type.name()) +
            " does not match stored element type " + std::string(this->type.elementType->name()) + ".\n" +
            this->toString()
        );
    }
}

bool Data::DataView::operator==(const DataView& other) const noexcept
{
    return (this->ptr == other.ptr) && (this->type == other.type);
}

bool Data::DataView::operator!=(const DataView& other) const noexcept
{
    return !(*this == other);
}

Data::DataView::operator bool() const noexcept {
    return this->ptr != nullptr;
}

size_t Data::DataView::scaleLocation(const Data::DataType& required, const size_t address) const {
    size_t scaleAddress = address % this->type.getAddressSpace(required);
    if(this->type.rank == 2) {
        scaleAddress = this->type.getValidAddress(required, scaleAddress);
    }
    return scaleAddress;
    
}

std::string Data::DataView::toString() const {
    std::ostringstream oss;

    oss << "DataView{\n"
        << "\tPointer   = " << ptr << ",\n"
        << "\tDataType  = " << type.toString() << "\n"
        << "}";

    return oss.str();
}
