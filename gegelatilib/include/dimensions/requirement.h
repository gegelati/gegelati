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
         */
        Requirement(Data::DataType dataType, const Constraint& constraint)
            : dataType(std::move(dataType)), constraint(constraint.cloneSharedPtr()) {}

        /**
         * \brief Creates a requirement with a value constraint.
         * \param[in] dataType Required shape and element type.
         * \param[in] constraint Constraint that values must satisfy as sharedPtr.
         */
        Requirement(Data::DataType dataType, const std::shared_ptr<const Constraint>& constraint)
            : dataType(std::move(dataType)), constraint(constraint) {}

        /** \brief Returns the required shape and element type. */
        const Data::DataType& getDataType() const noexcept {
            return this->dataType;
        }

        /**
         * \brief Returns whether a view matches the shape, type, and value constraint.
         * \param[in] view View to validate.
         * \return `true` when the view satisfies this requirement.
         */
        bool accepts(const Data::DataView& view) const {
            return view.getElementType() == *this->dataType.elementType && 
                   view.getType().canFitIn(this->dataType) &&
                   this->constraint->accepts(view);
        }

        /** \brief Checks whether this producer requirement satisfies a consumer requirement. */
        bool isCompatibleWith(const Requirement& consumer) const noexcept {
            return *this->dataType.elementType == *consumer.getDataType().elementType &&
                   this->dataType.canFitIn(consumer.dataType, 0) &&
                   this->constraint->isCompatibleWith(*consumer.constraint);
        }

        /** \brief Compares both structural type and value constraint.
         * \param[in] other Requirement to compare with.
         * \return `true` when both requirements are equivalent.
         */
        bool operator==(const Requirement& other) const noexcept {
            return this->dataType == other.dataType && this->constraint->equals(*other.constraint);
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
            return *this->constraint;
        }

        /** \brief Returns a diagnostic string containing the type and constraint. */
        std::string toString() const {
            return "Requirement{\n\t" + this->dataType.toString() + ",\n\tRequirement: " + this->constraint->toString() + "\n}";
        }

        /** \brief Renders the requirement in a compact model-summary format. */
        std::string summary() const {
            return this->dataType.summary() + " in " + this->constraint->toString();
        }

        /**
         * \brief Static method to control if a list of requirement accepts a list of views (1 to 1)
         */
        static bool acceptListViews(const std::vector<Requirement>& requirements, const std::vector<Data::DataView>& views) {
            if(requirements.size() != views.size()) {
                return false;
            }
            for(size_t idx = 0; idx < requirements.size(); idx++) {
                if(!requirements.at(idx).accepts(views.at(idx))) {
                    return false;
                }
            }
            return true;
        }

        /**
         * \brief Static method to control if a list of requirement accepts a list of values (1 to 1)
         */
        static bool acceptListValues(const std::vector<Requirement>& requirements, const std::vector<Data::DataValue>& values) {
            std::vector<Data::DataView> views;
            for(const Data::DataValue& value: values) {
                views.push_back(value.view());
            }
            return acceptListViews(requirements, views);
        };
    };


    /**
     * \brief operator for printing
     */
    inline std::ostream& operator<<(std::ostream& os, const Requirement& requirement) {
        return os << requirement.toString();
    }


} // namespace Dimensions

#endif // DATA_REQUIREMENT_H
