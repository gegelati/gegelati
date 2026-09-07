#include "data/dataValue.h"


Data::DataValue::DataValue(std::unique_ptr<detail::ValueConcept> storage, DataType type)
    : DataView(storage->data(), type),
        storage(std::move(storage)) { }



Data::DataView Data::DataValue::view() const
{
    return Data::DataView(this->storage->data(), this->type);
}

Data::DataValue Data::DataValue::clone() const
{
    return Data::DataValue(this->storage->clone(), this->type);
}


void Data::DataValue::setSubValue(const Data::DataValue& value, size_t address) {
    if (*type.elementType != *value.type.elementType) {
        throw std::runtime_error(
            "Data::DataValue::setSubValue failed: source and destination element types differ.\n"
            "Destination:\n" + this->toString() + "\n"
            "Source:\n" + value.toString()
        );
    }

    const DataType& requested = value.type;

    if (!this->canFit(requested, address)) {
        throw std::out_of_range(
            "Data::DataValue::setSubValue failed at address " + std::to_string(address) + ".\n"
            "Destination:\n" + this->toString() + "\n"
            "Source:\n" + value.toString()
        );
    }

    // Copy the source value into the destination while respecting 2D row strides.
    size_t offset = type.sourceOffset + address;

    const char* src = static_cast<const char*>(value.storage->data());
    char* dst = static_cast<char*>(storage->data());

    if (requested.rank == 0) {
        std::copy(
            src,
            src + type.elementSize,
            dst + offset * type.elementSize
        );
    }
    else if (requested.rank == 1) {
        const size_t count = requested.dimensions[0];

        std::copy(
            src,
            src + count * type.elementSize,
            dst + offset * type.elementSize
        );
    }
    else {
        const size_t rows = requested.dimensions[0];
        const size_t cols = requested.dimensions[1];
        const size_t stride = type.sourceDimensions[1];

        const size_t row = offset / stride;
        const size_t col = offset % stride;

        for (size_t r = 0; r < rows; ++r) {
            std::copy(
                src + r * cols * type.elementSize,
                src + (r + 1) * cols * type.elementSize,
                dst + ((row + r) * stride + col) * type.elementSize
            );
        }
    }
}

std::string Data::DataValue::toString() const {

    std::ostringstream oss;
    oss << "Data::DataValue{\n"
        << "\tValue     = " << storage->toString() << "\n"
        << "\tDataType  =  " << this->type.toString() <<",\n"
        << "}";

    return oss.str();
}

Data::DataValue::operator bool() const noexcept {
    return static_cast<bool>(storage);
}

bool Data::DataValue::operator==(const Data::DataValue& other) const noexcept
{
    return this->storage->equals(*other.storage);
}

bool Data::DataValue::operator!=(const Data::DataValue& other) const noexcept
{
    return !(*this == other);
}
