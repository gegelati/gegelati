#ifndef DATA_REQUIREMENT_H
#define DATA_REQUIREMENT_H

#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "newData/dataView.h"

namespace Data {

    /**
     * \brief Inclusive numeric bounds for one element type.
     *
     * Bounds are stored as T, rather than converted to a common type. This is important
     * for integer types whose values cannot be represented exactly by floating point.
     * 
     * A value is accepted if it is contained in the ranges OR EQUAL to the ranges 
     */
    template <typename T>
    struct NumericRange {
        static_assert(std::is_arithmetic_v<T>, "NumericRange requires an arithmetic type.");

        std::optional<T> minimum;
        std::optional<T> maximum;

        static NumericRange unbounded() {
            return {};
        }

        static NumericRange atLeast(T minimum) {
            return {minimum, std::nullopt};
        }

        static NumericRange atMost(T maximum) {
            return {std::nullopt, maximum};
        }

        static NumericRange between(T minimum, T maximum) {
            if (minimum > maximum) {
                throw std::invalid_argument("Numeric range minimum must not exceed maximum.");
            }
            return {minimum, maximum};
        }

        bool contains(const T& value) const noexcept {
            return (!minimum || value >= *minimum) &&
                   (!maximum || value <= *maximum);
        }
    };

    namespace detail {

        struct DataRequirementConcept {
            virtual ~DataRequirementConcept() = default;
            virtual bool accepts(const DataView& view) const = 0;
            virtual bool equals(const DataRequirementConcept& other) const = 0;
            virtual std::string toString() const = 0;
        };

        struct UnconstrainedDataRequirement final : DataRequirementConcept {
            bool accepts(const DataView&) const override {
                return true;
            }

            bool equals(const DataRequirementConcept& other) const override {
                return dynamic_cast<const UnconstrainedDataRequirement*>(&other) != nullptr;
            }

            std::string toString() const override {
                return "unconstrained";
            }
        };

        template <typename T>
        struct NumericRangeRequirement final : DataRequirementConcept {
            NumericRange<T> range;

            explicit NumericRangeRequirement(NumericRange<T> range)
                : range(std::move(range)) {}

            bool accepts(const DataView& view) const override {
                if (!view || !view.getType().elementType ||
                    *view.getType().elementType != typeid(T)) {
                    return false;
                }

                // If scalar, return if scalar is contained
                if(view.getRank() == 0) {
                    return range.contains(view.getScalar<T>());

                // If array, check if every value is contained
                } else {
                    const T* values = view.getArray<T>();
                    size_t nbValues = view.getType().totalElements();
                    for(size_t idx = 0; idx < nbValues; idx ++) {
                        if (!range.contains(values[idx])) {
                            return false;
                        }
                    }
                }
                return true;
            }

            bool equals(const DataRequirementConcept& other) const override {
                const auto* numeric = dynamic_cast<const NumericRangeRequirement*>(&other);
                return numeric != nullptr && range.minimum == numeric->range.minimum &&
                       range.maximum == numeric->range.maximum;
            }

            std::string toString() const override {
                std::ostringstream result;
                result << "Numeric Range: [";
                if (range.minimum) {
                    result << *range.minimum;
                } else {
                    result << "-inf";
                }
                result << ", ";
                if (range.maximum) {
                    result << *range.maximum;
                } else {
                    result << "+inf";
                }
                result << "]";
                return result.str();
            }
        };

    } // namespace detail

    /**
     * \brief Describes the shape, element type, and accepted value domain of data.
     *
     * DataType remains a structural descriptor. DataRequirement composes it with an
     * optional type-erased constraint, allowing numeric constraints without imposing
     * comparison operators on arbitrary DataType values.
     */
    class DataRequirement {
    public:
        using Concept = detail::DataRequirementConcept;

    private:
        DataType dataType;
        std::shared_ptr<const Concept> constraint;

        DataRequirement(DataType dataType, std::shared_ptr<const Concept> constraint)
            : dataType(std::move(dataType)), constraint(std::move(constraint)) {}


    public:

        /** \brief default constructor */
        DataRequirement(): constraint(std::make_shared<const detail::UnconstrainedDataRequirement>()) {}

        /** \brief Creates a requirement with no value constraint. */
        explicit DataRequirement(DataType dataType)
            : dataType(std::move(dataType)),
              constraint(std::make_shared<const detail::UnconstrainedDataRequirement>()) {}

        template <typename T>
        DataRequirement(DataType dataType, NumericRange<T> range)
            : dataType(std::move(dataType)), constraint(std::make_shared<const detail::NumericRangeRequirement<T>>(std::move(range))) {
            if (!dataType.elementType || *dataType.elementType != typeid(T)) {
                throw std::invalid_argument("Numeric range type must match the DataType element type.");
            }
        }

        template <typename T>
        static DataRequirement scalar() {
            return DataRequirement(DataType::scalar<T>());
        }

        template <typename T>
        static DataRequirement array1d(size_t size) {
            return DataRequirement(DataType::array1d<T>(size));
        }

        template <typename T>
        static DataRequirement array2d(size_t rows, size_t columns) {
            return DataRequirement(DataType::array2d<T>(rows, columns));
        }

        template <typename T>
        static DataRequirement scalar(NumericRange<T> range) {
            return DataRequirement(DataType::scalar<T>(), std::move(range));
        }

        template <typename T>
        static DataRequirement array1d(size_t size, NumericRange<T> range) {
            return DataRequirement(DataType::array1d<T>(size), std::move(range));
        }

        template <typename T>
        static DataRequirement array2d(size_t rows, size_t columns, NumericRange<T> range) {
            return DataRequirement(DataType::array2d<T>(rows, columns), std::move(range));
        }

        const DataType& getDataType() const noexcept {
            return dataType;
        }

        /** \brief Returns whether a view matches the shape, type, and value constraint. */
        bool accepts(const DataView& view) const {
            return view.getType() == dataType && constraint->accepts(view);
        }

        bool operator==(const DataRequirement& other) const noexcept {
            return dataType == other.dataType && constraint->equals(*other.constraint);
        }

        bool operator!=(const DataRequirement& other) const noexcept {
            return !(*this == other);
        }

        std::string toString() const {
            return "DataRequirement{\n\t" + dataType.toString() + ",\n\tRequirement: " +constraint->toString() + "\n}";
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const DataRequirement& requirement) {
        return os << requirement.toString();
    }

} // namespace Data

#endif // DATA_REQUIREMENT_H
