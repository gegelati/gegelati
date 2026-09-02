#ifndef NUMERIC_RANGE_H
#define NUMERIC_RANGE_H

#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "newData/dataValue.h"

namespace Data {

    template <typename T>
    struct NumericRange final : DataConstraint {
        /// \brief Numeric type constrained by this range.
        static_assert(std::is_arithmetic_v<T>, "NumericRange requires an arithmetic type.");

        /// \brief Inclusive lower bound, or no lower bound.
        std::optional<T> minimum;
        /// \brief Inclusive upper bound, or no upper bound.
        std::optional<T> maximum;

        /// \brief Constructs an unbounded numeric range.
        NumericRange() = default;

        /**
         * \brief Constructs a range from optional inclusive bounds.
         * \param[in] minimum Optional inclusive lower bound.
         * \param[in] maximum Optional inclusive upper bound.
         */
        NumericRange(std::optional<T> minimum, std::optional<T> maximum)
            : minimum(std::move(minimum)), maximum(std::move(maximum)) {}

        /// \brief Returns an unbounded range.
        static NumericRange unbounded() {
            return {};
        }

        /** \brief Returns a range with an inclusive lower bound.
         * \param[in] minimum Inclusive lower bound.
         */
        static NumericRange atLeast(T minimum) {
            return {minimum, std::nullopt};
        }

        /** \brief Returns a range with an inclusive upper bound.
         * \param[in] maximum Inclusive upper bound.
         */
        static NumericRange atMost(T maximum) {
            return {std::nullopt, maximum};
        }

        /** \brief Returns a range with inclusive lower and upper bounds.
         * \param[in] minimum Inclusive lower bound.
         * \param[in] maximum Inclusive upper bound.
         * \throws std::invalid_argument If minimum exceeds maximum.
         */
        static NumericRange between(T minimum, T maximum) {
            if (minimum > maximum) {
                throw std::invalid_argument(
                    "NumericRange::between failed: minimum (" + std::to_string(minimum) +
                    ") must not exceed maximum (" + std::to_string(maximum) + ")."
                );
            }
            return {minimum, maximum};
        }

        /** \brief Checks whether a value is within both inclusive bounds.
         * \param[in] value Value to test.
         * \return `true` when the value is accepted.
         */
        bool contains(const T& value) const noexcept {
            return (!minimum || value >= *minimum) &&
                   (!maximum || value <= *maximum);
        }

        /** \brief Checks whether every element in a view is within this range.
         * \param[in] view View whose elements are checked.
         * \return `true` when the view has type T and all values are accepted.
         */
        bool accepts(const DataView& view) const override {
            if (!view || !view.getType().elementType ||
                *view.getType().elementType != typeid(T)) {
                return false;
            }

            if (view.getRank() == 0) {
                return contains(view.getScalar<T>());
            }

            const T* values = view.getArray<T>();
            for (size_t index = 0; index < view.getType().totalElements(); ++index) {
                if (!contains(values[index])) {
                    return false;
                }
            }
            return true;
        }

        /**
         * \brief Clamps a value to this range.
         *
         * The input must already have element type `T`; `DataValue::convert<T>`
         * performs the type conversion before this method is called.
         *
         * \param[in] value Value whose elements are clamped.
         * \return A newly allocated value with the same rank and dimensions.
         * \throws std::invalid_argument If the value has another element type or rank.
         */
        std::unique_ptr<DataValue> convert(const DataValue& value) const override {
            if (!value || value.getElementType() != typeid(T)) {
                throw std::invalid_argument(
                    "NumericRange::convert failed: value has an incompatible element type."
                );
            }

            auto clamp = [this](T current) {
                if (minimum && current < *minimum) {
                    return *minimum;
                }
                if (maximum && current > *maximum) {
                    return *maximum;
                }
                return current;
            };

            if (value.getRank() == 0) {
                return std::make_unique<DataValue>(
                    DataValue::scalar<T>(clamp(value.getScalar<T>())));
            }

            const size_t count = value.getType().totalElements();
            const T* source = value.getArray<T>();
            auto values = std::make_unique<T[]>(count);
            for (size_t index = 0; index < count; ++index) {
                values[index] = clamp(source[index]);
            }

            if (value.getRank() == 1) {
                return std::make_unique<DataValue>(
                    DataValue::array1d<T>(std::move(values), value.getDimensions()[0]));
            }
            if (value.getRank() == 2) {
                return std::make_unique<DataValue>(
                    DataValue::array2d<T>(std::move(values), value.getDimensions()[0],
                                          value.getDimensions()[1]));
            }

            throw std::invalid_argument(
                "NumericRange::convert failed: unsupported value rank."
            );
        }

        /** \brief Compares this range with another constraint.
         * \param[in] other Constraint to compare with.
         * \return `true` when the other constraint has equal bounds and type.
         */
        bool equals(const DataConstraint& other) const override {
            const auto* numeric = dynamic_cast<const NumericRange*>(&other);
            return numeric != nullptr && minimum == numeric->minimum &&
                   maximum == numeric->maximum;
        }

        /// \brief Returns a shared deep copy of this range.
        std::shared_ptr<const DataConstraint> cloneSharedPtr() const override {
            return std::make_shared<const NumericRange<T>>(*this);
        }

        /// \brief Returns the inclusive bounds as a diagnostic string.
        std::string toString() const override {
            std::ostringstream result;
            result << "Numeric Range: [";
            if (minimum) {
                result << *minimum;
            } else {
                result << "-inf";
            }
            result << ", ";
            if (maximum) {
                result << *maximum;
            } else {
                result << "+inf";
            }
            result << "]";
            return result.str();
        }
    };

} // namespace Data

#endif // NUMERIC_RANGE_H
