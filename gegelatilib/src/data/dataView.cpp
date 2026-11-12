#include "data/dataView.h"


bool Data::DataView::canFit(const DataType& requested, size_t address) const noexcept {
    if (requested.elementType == nullptr || this->type.elementType == nullptr) {
        return false;
    }
    if (!sameElementType(*requested.elementType, *this->type.elementType)) {
        return false;
    }
    return this->type.canFitIn(requested, address);
}

Data::DataView Data::DataView::getSubView(const DataType& requested, size_t address) const {
    if (!this->canFit(requested, address)) {
        throw std::out_of_range(
            "Data::DataView::getSubView failed at address " + std::to_string(address) + ".\n"
            "Current view:\n" + this->toString() + "\n"
            "Requested type:\n" + requested.toString()
        );
    }

    // The window starts at the position in memory of the address, and follows the strides of this view.
    const size_t byteOffset = this->type.offsetOf(address) * this->type.elementSize;

    return DataView(
        static_cast<const char*>(this->ptr) + byteOffset,
        DataType::subView(requested, this->type)
    );
}

void Data::DataView::canBeAccess(const std::type_info& requestedType) const {
    if (!this->ptr) {
        throw std::runtime_error(
            "DataView access failed: the view has no data pointer.\n" + this->toString()
        );
    }
    if (!sameElementType(*this->type.elementType, requestedType)) {
        throw std::runtime_error(
            "DataView access failed: requested element type " + std::string(requestedType.name()) +
            " does not match stored element type " + std::string(this->type.elementType->name()) + ".\n" +
            this->toString()
        );
    }
}

std::unique_ptr<std::byte[]> Data::DataView::deepPtrClone() const
{
    // Copy the memory covered by the view, from its first to its last element. The layout is kept by the clone.
    const size_t bytes = this->type.memorySpan() * this->type.elementSize;

    auto storage = std::make_unique<std::byte[]>(bytes);

    const auto* source =
        static_cast<const std::byte*>(this->ptr);

    std::memcpy(storage.get(), source, bytes);

    return storage;
}

std::pair<std::unique_ptr<std::byte[]>, Data::DataView> Data::DataView::deepClone() const
{
    std::unique_ptr<std::byte[]> clonePtr = this->deepPtrClone();
    Data::DataView cloneView(clonePtr.get(), this->type);
    return {std::move(clonePtr), std::move(cloneView)};
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
    if(this->type.rank >= 2) {
        scaleAddress = this->type.getValidAddress(required, scaleAddress);
    }
    return scaleAddress;
    
}

std::string Data::DataView::toString(std::string prefix) const {
    std::ostringstream oss;

    oss << prefix << "DataView{\n"
        << prefix << "\tPointer   = " << this->ptr << ",\n"
        << prefix << "\tDataType  = " << this->type.toString() << "\n"
        << prefix << "}";

    return oss.str();
}

size_t Data::DataView::getCombinedHash(const std::vector<DataView>& views)
{
    size_t hash = 0;

    for (const auto& view : views) {
        const std::uint64_t h = view.hash();

        hash ^= h + 0x9e3779b97f4a7c15ULL +
                (hash << 6) +
                (hash >> 2);
    }

    return hash;
}

// Hash of a block of bytes, reading 8 bytes at a time.
// The 4 lanes are independent so that the multiplications can overlap, which makes it several times
// faster than a byte by byte hash (e.g. FNV-1a) on big arrays.
uint64_t Data::DataView::hashBytes(const unsigned char* bytes, size_t size) noexcept
{
    constexpr uint64_t prime = 0x9E3779B97F4A7C15ull;
    auto mix = [](uint64_t lane, uint64_t word) {
        lane = (lane ^ word) * prime;
        return lane ^ (lane >> 29);
    };

    uint64_t lanes[4] = {0x243F6A8885A308D3ull, 0x13198A2E03707344ull, 0xA4093822299F31D0ull, 0x082EFA98EC4E6C89ull};
    size_t idx = 0;
    uint64_t words[4];

    for (; idx + 32 <= size; idx += 32) {
        std::memcpy(words, bytes + idx, 32);
        for (size_t lane = 0; lane < 4; ++lane) {
            lanes[lane] = mix(lanes[lane], words[lane]);
        }
    }

    // Last bytes, one by one.
    uint64_t hash = mix(mix(lanes[0], lanes[1]), mix(lanes[2], lanes[3]));
    for (; idx < size; ++idx) {
        hash = mix(hash, bytes[idx]);
    }
    return mix(hash, size);
}

size_t Data::DataView::hash() const
{
    // The hash only depends on the shape and on the values of the elements, not on where they are in memory:
    // a window of a bigger array has the same hash as a copy of the window.
    uint64_t hash = 0;
    if (this->ptr != nullptr) {
        const size_t totalBytes = this->type.totalElements() * this->type.elementSize;
        if (this->type.isContiguous()) {
            hash = hashBytes(static_cast<const unsigned char*>(this->ptr), totalBytes);
        } else {
            std::vector<unsigned char> compact(totalBytes);
            const size_t elementSize = this->type.elementSize;
            const auto* source = static_cast<const unsigned char*>(this->ptr);
            this->type.forEachRun(this->type, 0, [&](size_t memoryOffset, size_t blockOffset, size_t length) {
                std::memcpy(compact.data() + blockOffset * elementSize, source + memoryOffset * elementSize, length * elementSize);
            });
            hash = hashBytes(compact.data(), totalBytes);
        }
    }

    // Arrays of the same values with another shape are different data.
    for (size_t axis = 0; axis < this->type.rank; ++axis) {
        hash = (hash ^ this->type.dimensions[axis]) * 0x100000001B3ull;
    }
    return static_cast<size_t>(hash);
}