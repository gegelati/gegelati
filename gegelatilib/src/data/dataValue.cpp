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
    if (!Data::sameElementType(*this->type.elementType, *value.type.elementType)) {
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

    // Copy the source value run by run, respecting the strides of the destination layout.
    const size_t base = this->type.offsetOf(address);
    const size_t elementSize = this->type.elementSize;

    const char* src = static_cast<const char*>(value.storage->data());
    char* dst = static_cast<char*>(this->storage->data());

    this->type.forEachRun(requested, base, [&](size_t dstOffset, size_t srcOffset, size_t length) {
        std::copy(
            src + srcOffset * elementSize,
            src + (srcOffset + length) * elementSize,
            dst + dstOffset * elementSize
        );
    });
}

std::string Data::DataValue::toString(std::string prefix) const {

    std::ostringstream oss;
    oss << prefix << "Data::DataValue{\n"
        << prefix << "\tValue     = " << this->storage->toString() << "\n"
        << prefix << "\tDataType  =  " << this->type.toString() <<",\n"
        << prefix << "}";

    return oss.str();
}

std::string Data::DataValue::toShortString(std::string prefix) const {
    std::ostringstream oss;
    oss << prefix << "Data=" << this->storage->toString() << ", type=" << DEMANGLE_TYPEID_NAME(this->getElementType().name()) << "";
    return oss.str();
}

std::string Data::DataValue::toValueString() const {
    return this->storage->toString();
}

Data::DataValue::operator bool() const noexcept {
    return static_cast<bool>(this->storage);
}

bool Data::DataValue::operator==(const Data::DataValue& other) const noexcept
{
    return this->storage->equals(*other.storage);
}

bool Data::DataValue::operator!=(const Data::DataValue& other) const noexcept
{
    return !(*this == other);
}

size_t Data::DataValue::getCombinedHash(const std::vector<Data::DataValue>& values)
{
    std::vector<Data::DataView> views;
    for(const Data::DataValue& value: values) {
        views.push_back(value.view());
    }
    return Data::DataView::getCombinedHash(views);
}