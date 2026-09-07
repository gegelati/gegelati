#ifndef DATA_REQUIREMENT_H
#define DATA_REQUIREMENT_H

#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "data/dataView.h"
#include "dimensions/constraint.h"

namespace Dimensions {


    /**
     * \brief Describes the shape, element type, and accepted value domain of data.
     *
     * DataType remains a structural descriptor. Requirement composes it with an
     * optional type-erased constraint, allowing numeric constraints without imposing
     * comparison operators on arbitrary DataType values.
     */
    class Requirement {
    public:

    private:
        /// \brief Structural type required by this requirement.
        Data::DataType dataType;
        /// \brief Value constraint applied after the structural type check.
        std::shared_ptr<const Constraint> constraint;

    public:

        /** \brief Constructs an unconstrained requirement with invalid default type metadata. */
        Requirement(): constraint(std::make_shared<const UnconstrainedData>()) {}

        /** \brief Creates a requirement with no value constraint.
         * \param[in] dataType Required shape and element type.
         */
        explicit Requirement(Data::DataType dataType)
            : dataType(std::move(dataType)),
                            constraint(std::make_shared<const UnconstrainedData>()) {}

        /**
         * \brief Creates a requirement with a value constraint.
         * \param[in] dataType Required shape and element type.
         * \param[in] constraint Constraint that values must satisfy.
         * \throws std::invalid_argument If the constraint cannot be cloned.
         */
        Requirement(Data::DataType dataType, const Constraint& constraint)
            : dataType(std::move(dataType)), constraint(constraint.cloneSharedPtr()) {}

        template <typename T>
        /** \brief Creates an unconstrained scalar requirement of type T. */
        static Requirement scalar() {
            return Requirement(Data::DataType::scalar<T>());
        }

        template <typename T>
        /** \brief Creates an unconstrained 1D array requirement of type T.
         * \param[in] size Required number of elements.
         */
        static Requirement array1d(size_t size) {
            return Requirement(Data::DataType::array1d<T>(size));
        }

        template <typename T>
        /** \brief Creates an unconstrained 2D array requirement of type T.
         * \param[in] rows Required number of rows.
         * \param[in] columns Required number of columns.
         */
        static Requirement array2d(size_t rows, size_t columns) {
            return Requirement(Data::DataType::array2d<T>(rows, columns));
        }

        template <typename T>
        /** \brief Creates a constrained scalar requirement of type T.
         * \param[in] constraint Value constraint to apply.
         */
        static Requirement scalar(const Constraint& constraint) {
            return Requirement(
                Data::DataType::scalar<T>(),
                constraint);
        }

        template <typename T>
        /** \brief Creates a constrained 1D array requirement of type T.
         * \param[in] size Required number of elements.
         * \param[in] constraint Value constraint to apply.
         */
        static Requirement array1d(size_t size, const Constraint& constraint) {
            return Requirement(
                Data::DataType::array1d<T>(size),
                constraint);
        }

        template <typename T>
        /** \brief Creates a constrained 2D array requirement of type T.
         * \param[in] rows Required number of rows.
         * \param[in] columns Required number of columns.
         * \param[in] constraint Value constraint to apply.
         */
        static Requirement array2d(size_t rows, size_t columns, const Constraint& constraint) {
            return Requirement(
                Data::DataType::array2d<T>(rows, columns),
                constraint);
        }

        /** \brief Returns the required shape and element type. */
        const Data::DataType& getDataType() const noexcept {
            return dataType;
        }

        /**
         * \brief Returns whether a view matches the shape, type, and value constraint.
         * \param[in] view View to validate.
         * \return `true` when the view satisfies this requirement.
         */
        bool accepts(const Data::DataView& view) const {
            return view.getType() == dataType && constraint->accepts(view);
        }

        /** \brief Checks whether this producer requirement satisfies a consumer requirement. */
        bool isCompatibleWith(const Requirement& consumer) const noexcept {
            return dataType.elementType == consumer.getDataType().elementType && 
                   dataType.canFitIn(consumer.dataType, 0) &&
                   constraint->isCompatibleWith(*consumer.constraint);
        }

        /** \brief Compares both structural type and value constraint.
         * \param[in] other Requirement to compare with.
         * \return `true` when both requirements are equivalent.
         */
        bool operator==(const Requirement& other) const noexcept {
            return dataType == other.dataType && constraint->equals(*other.constraint);
        }

        /** \brief Checks whether two requirements differ.
         * \param[in] other Requirement to compare with.
         * \return `true` when the requirements are not equivalent.
         */
        bool operator!=(const Requirement& other) const noexcept {
            return !(*this == other);
        }

        /** \brief Returns the value constraint associated with this requirement. */
        const Constraint& getConstraint() const noexcept {
            return *constraint;
        }

        /** \brief Returns a diagnostic string containing the type and constraint. */
        std::string toString() const {
            return "Requirement{\n\t" + dataType.toString() + ",\n\tRequirement: " + constraint->toString() + "\n}";
        }

        /** \brief Renders the requirement in a compact model-summary format. */
        std::string summary() const {
            return dataType.summary() + " in " + constraint->toString();
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const Requirement& requirement) {
        return os << requirement.toString();
    }
    inline std::ostream& operator<<(std::ostream& os, const Constraint& cosntraint) {
        return os << cosntraint.toString();
    }

} // namespace Data

#endif // DATA_REQUIREMENT_H
