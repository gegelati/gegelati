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

    class DataValue;

    /**
     * \brief Inclusive numeric bounds for one element type.
     *
     * Bounds are stored as T, rather than converted to a common type. This is important
     * for integer types whose values cannot be represented exactly by floating point.
     * 
     * A value is accepted if it is contained in the ranges OR EQUAL to the ranges 
     */
    struct DataConstraint {
        /// \brief Destroys the constraint through its polymorphic interface.
        virtual ~DataConstraint() = default;
        /** \brief Checks whether a view satisfies this constraint.
         * \param[in] view View whose value is checked.
         * \return `true` when the view is accepted.
         */
        virtual bool accepts(const DataView& view) const = 0;
        /**
         * \brief Converts a value to the type represented by this constraint.
         *
         * \param[in] value Value to convert.
         * \return A newly allocated converted value.
         * \throws std::invalid_argument If conversion is unsupported or the value
         * cannot be repaired by this constraint.
         */
        virtual std::unique_ptr<DataValue> convert(const DataValue& value) const = 0;
        /** \brief Compares this constraint with another constraint.
         * \param[in] other Constraint to compare with.
         * \return `true` when both constraints have the same semantics.
         */
        virtual bool equals(const DataConstraint& other) const = 0;
        /// \brief Creates a shared deep copy of this constraint.
        virtual std::shared_ptr<const DataConstraint> cloneSharedPtr() const = 0;
        /// \brief Returns a diagnostic description of this constraint.
        virtual std::string toString() const = 0;
    };

    struct UnconstrainedData final : DataConstraint {
        /// \brief Accepts every view.
        bool accepts(const DataView&) const override {
            return true;
        }

        /**
         * \brief Returns an unchanged deep copy of the supplied value.
         *
         * \param[in] value Value to copy.
         * \return A newly allocated value with equivalent data and metadata.
         */
        std::unique_ptr<DataValue> convert(const DataValue& value) const override;

        /** \brief Compares this unconstrained marker with another constraint.
         * \param[in] other Constraint to compare with.
         * \return `true` only when the other constraint is also unconstrained.
         */
        bool equals(const DataConstraint& other) const override {
            return dynamic_cast<const UnconstrainedData*>(&other) != nullptr;
        }

        /// \brief Creates a shared copy of the unconstrained marker.
        std::shared_ptr<const DataConstraint> cloneSharedPtr() const override {
            return std::make_shared<const UnconstrainedData>(*this);
        }

        /// \brief Returns the unconstrained diagnostic label.
        std::string toString() const override {
            return "unconstrained";
        }
    };

    /**
     * \brief Describes the shape, element type, and accepted value domain of data.
     *
     * DataType remains a structural descriptor. DataRequirement composes it with an
     * optional type-erased constraint, allowing numeric constraints without imposing
     * comparison operators on arbitrary DataType values.
     */
    class DataRequirement {
    public:

    private:
        /// \brief Structural type required by this requirement.
        DataType dataType;
        /// \brief Value constraint applied after the structural type check.
        std::shared_ptr<const DataConstraint> constraint;

    public:

        /** \brief Constructs an unconstrained requirement with invalid default type metadata. */
        DataRequirement(): constraint(std::make_shared<const UnconstrainedData>()) {}

        /** \brief Creates a requirement with no value constraint.
         * \param[in] dataType Required shape and element type.
         */
        explicit DataRequirement(DataType dataType)
            : dataType(std::move(dataType)),
                            constraint(std::make_shared<const UnconstrainedData>()) {}

        /**
         * \brief Creates a requirement with a value constraint.
         * \param[in] dataType Required shape and element type.
         * \param[in] constraint Constraint that values must satisfy.
         * \throws std::invalid_argument If the constraint cannot be cloned.
         */
        DataRequirement(DataType dataType, const DataConstraint& constraint)
            : dataType(std::move(dataType)), constraint(constraint.cloneSharedPtr()) {
            if (!this->constraint) {
                throw std::invalid_argument(
                    "DataRequirement construction failed: constraint clone is null.\n"
                    "Requirement type:\n" + this->dataType.toString()
                );
            }
        }

        template <typename T>
        /** \brief Creates an unconstrained scalar requirement of type T. */
        static DataRequirement scalar() {
            return DataRequirement(DataType::scalar<T>());
        }

        template <typename T>
        /** \brief Creates an unconstrained 1D array requirement of type T.
         * \param[in] size Required number of elements.
         */
        static DataRequirement array1d(size_t size) {
            return DataRequirement(DataType::array1d<T>(size));
        }

        template <typename T>
        /** \brief Creates an unconstrained 2D array requirement of type T.
         * \param[in] rows Required number of rows.
         * \param[in] columns Required number of columns.
         */
        static DataRequirement array2d(size_t rows, size_t columns) {
            return DataRequirement(DataType::array2d<T>(rows, columns));
        }

        template <typename T>
        /** \brief Creates a constrained scalar requirement of type T.
         * \param[in] constraint Value constraint to apply.
         */
        static DataRequirement scalar(const DataConstraint& constraint) {
            return DataRequirement(
                DataType::scalar<T>(),
                constraint);
        }

        template <typename T>
        /** \brief Creates a constrained 1D array requirement of type T.
         * \param[in] size Required number of elements.
         * \param[in] constraint Value constraint to apply.
         */
        static DataRequirement array1d(size_t size, const DataConstraint& constraint) {
            return DataRequirement(
                DataType::array1d<T>(size),
                constraint);
        }

        template <typename T>
        /** \brief Creates a constrained 2D array requirement of type T.
         * \param[in] rows Required number of rows.
         * \param[in] columns Required number of columns.
         * \param[in] constraint Value constraint to apply.
         */
        static DataRequirement array2d(size_t rows, size_t columns, const DataConstraint& constraint) {
            return DataRequirement(
                DataType::array2d<T>(rows, columns),
                constraint);
        }

        /** \brief Returns the required shape and element type. */
        const DataType& getDataType() const noexcept {
            return dataType;
        }

        /**
         * \brief Returns whether a view matches the shape, type, and value constraint.
         * \param[in] view View to validate.
         * \return `true` when the view satisfies this requirement.
         */
        bool accepts(const DataView& view) const {
            return view.getType() == dataType && constraint->accepts(view);
        }

        /** \brief Compares both structural type and value constraint.
         * \param[in] other Requirement to compare with.
         * \return `true` when both requirements are equivalent.
         */
        bool operator==(const DataRequirement& other) const noexcept {
            return dataType == other.dataType && constraint->equals(*other.constraint);
        }

        /** \brief Checks whether two requirements differ.
         * \param[in] other Requirement to compare with.
         * \return `true` when the requirements are not equivalent.
         */
        bool operator!=(const DataRequirement& other) const noexcept {
            return !(*this == other);
        }

        /** \brief Returns the value constraint associated with this requirement. */
        const DataConstraint& getConstraint() const noexcept {
            return *constraint;
        }

        /** \brief Returns a diagnostic string containing the type and constraint. */
        std::string toString() const {
            return "DataRequirement{\n\t" + dataType.toString() + ",\n\tRequirement: " +constraint->toString() + "\n}";
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const DataRequirement& requirement) {
        return os << requirement.toString();
    }

} // namespace Data

#endif // DATA_REQUIREMENT_H
