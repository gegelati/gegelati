#ifndef NUMERIC_RANGE_H
#define NUMERIC_RANGE_H

#include <atomic>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "dimensions/constraint.h"
#include "rng/rng.h"
namespace Dimensions {

    /**
     * \brief intermediate class for cast detection with any type T
     */
    struct NumericRangeBase : Constraint {};

    /**
     * \brief Constraint structure for numeric types storing ranges of accepted numbers
     */
    template <typename T>
    struct NumericRange final : NumericRangeBase {
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
            return (!this->minimum || value >= *this->minimum) &&
                   (!this->maximum || value <= *this->maximum);
        }

        /** \brief Checks whether every element in a view is within this range.
         * \param[in] view View whose elements are checked.
         * \return `true` when the view has type T and all values are accepted.
         */
        bool accepts(const Data::DataView& view) const override {
            if (!view || !view.getType().elementType ||
                *view.getType().elementType != typeid(T)) {
                return false;
            }

            if (view.getRank() == 0) {
                return this->contains(view.getScalar<T>());
            }

            auto values = view.getData<T>();
            for (size_t index = 0; index < view.getType().totalElements(); ++index) {
                if (!this->contains(values[index])) {
                    return false;
                }
            }
            return true;
        }

        /** \brief Checks whether this producer range is contained by a consumer range. */
        bool isCompatibleWith(const Constraint& consumer) const override {
            const auto* numeric = dynamic_cast<const NumericRange<T>*>(&consumer);
            if (!numeric) {
                return dynamic_cast<const UnconstrainedData*>(&consumer) != nullptr;
            }

            const bool minimumCompatible = !numeric->minimum ||
                (this->minimum && *this->minimum >= *numeric->minimum);
            const bool maximumCompatible = !numeric->maximum ||
                (this->maximum && *this->maximum <= *numeric->maximum);
            return minimumCompatible && maximumCompatible;
        }

        /** \brief Compares this range with another constraint.
         * \param[in] other Constraint to compare with.
         * \return `true` when the other constraint has equal bounds and type.
         */
        bool equals(const Constraint& other) const override {
            const auto* numeric = dynamic_cast<const NumericRange*>(&other);
            return numeric != nullptr && this->minimum == numeric->minimum &&
                   this->maximum == numeric->maximum;
        }

        /// \brief Returns a shared deep copy of this range.
        std::shared_ptr<const Constraint> cloneSharedPtr() const override {
            return std::make_shared<const NumericRange<T>>(*this);
        }

        /**
         * Inherited from Constraint, samples uniformly in the range. A missing bound takes the value of the other bound, or 0 if both are
         * missing, so the generator of an unbounded range gives 0. A warning is written once per type.
         */
        std::unique_ptr<DataValueGenerator> makeGenerator() const override {
            static std::atomic<bool> warned{false};
            if ((!this->minimum || !this->maximum) && !warned.exchange(true)) {
                std::cerr << "Warning: Dimensions::NumericRange::makeGenerator: " << this->toString()
                          << " has a missing bound, the generator uses the other bound or 0. Give a generator to get other values." << std::endl;
            }
            const T low = this->minimum.value_or(this->maximum.value_or(T{}));
            return std::make_unique<NumericUniformGenerator<T>>(low, this->maximum.value_or(low));
        }

        /// \brief Returns the inclusive bounds as a diagnostic string.
        std::string toString() const override {
            std::ostringstream result;
            result << "Numeric Range: [";
            if (this->minimum) {
                result << *this->minimum;
            } else {
                result << "-inf";
            }
            result << ", ";
            if (this->maximum) {
                result << *this->maximum;
            } else {
                result << "+inf";
            }
            result << "] of type: " << DEMANGLE_TYPEID_NAME(typeid(T).name());

            return result.str();
        }
    };

} // namespace Data

#endif // NUMERIC_RANGE_H
