#include "newData/dataValue.h"

namespace Data {

    DataValue::DataValue(std::unique_ptr<Concept> storage, DataType type)
        : DataView(storage->data(), type),
          storage(std::move(storage)) {}

    void DataValue::setSubValue(const DataValue& value, size_t address) {
        if (*getType().elementType != *value.getType().elementType) {
            throw std::runtime_error("Type mismatch in setSubValue");
        }

        const DataType& requested = value.getType();

        if (!this->canFit(requested, address)) {
            throw std::out_of_range("Requested shape does not fit at the given address");
        }

        size_t offset = getType().sourceOffset + address;

        const char* src = static_cast<const char*>(value.storage->data());
        char* dst = static_cast<char*>(storage->data());

        if (requested.rank == 0) {
            std::copy(
                src,
                src + getType().elementSize,
                dst + offset * getType().elementSize
            );
        }
        else if (requested.rank == 1) {
            const size_t count = requested.dimensions[0];

            std::copy(
                src,
                src + count * getType().elementSize,
                dst + offset * getType().elementSize
            );
        }
        else if (requested.rank == 2) {
            const size_t rows = requested.dimensions[0];
            const size_t cols = requested.dimensions[1];
            const size_t stride = getType().sourceDimensions[1];

            const size_t row = offset / stride;
            const size_t col = offset % stride;

            for (size_t r = 0; r < rows; ++r) {
                std::copy(
                    src + r * cols * getType().elementSize,
                    src + (r + 1) * cols * getType().elementSize,
                    dst + ((row + r) * stride + col) * getType().elementSize
                );
            }
        }
        else {
            throw std::invalid_argument("Unsupported rank");
        }
    }

    std::string DataValue::toString() const {
        if (!storage) {
            return "DataValue{invalid}";
        }

        std::ostringstream oss;
        oss << "DataValue{\n"
            << "\tValue     = " << storage->toString() << "\n"
            << "\tDataType  =  " << this->getType().toString() <<",\n"
            << "}";

        return oss.str();
    }

    DataValue::operator bool() const noexcept {
        return static_cast<bool>(storage);
    }

} // namespace Data
