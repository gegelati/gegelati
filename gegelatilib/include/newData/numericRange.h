#ifndef NUMERIC_RANGE_H
#define NUMERIC_RANGE_H

#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "newData/dataView.h"
#include "newData/dataRequirement.h"

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
