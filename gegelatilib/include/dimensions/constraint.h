#ifndef DATA_CONSTRAINT_H
#define DATA_CONSTRAINT_H

#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include "data/dataValue.h"
#include "dimensions/dataValueGenerator.h"


namespace Dimensions {



    /**
     * \brief Abstract constraint on the values a data can take (e.g. a NumericRange), checked on top of its DataType.
     */
    struct Constraint {
        /// \brief Destroys the constraint through its polymorphic interface.
        virtual ~Constraint() = default;
        /** \brief Checks whether a view satisfies this constraint.
         * \param[in] view View whose value is checked.
         * \return `true` when the view is accepted.
         */
        virtual bool accepts(const Data::DataView& view) const = 0;
        /** \brief Checks whether this producer constraint satisfies a consumer constraint. */
        virtual bool isCompatibleWith(const Constraint& consumer) const =0;
        /** \brief Compares this constraint with another constraint.
         * \param[in] other Constraint to compare with.
         * \return `true` when both constraints have the same semantics.
         */
        virtual bool equals(const Constraint& other) const = 0;
        /// \brief Creates a shared deep copy of this constraint.
        virtual std::shared_ptr<const Constraint> cloneSharedPtr() const = 0;
        /// \brief Returns a diagnostic description of this constraint.
        virtual std::string toString() const = 0;

        /**
         * \brief Return the naive generator of the constraint: a generator sampling uniformly the values it accepts.
         *
         * \throw std::runtime_error if no generator can be made from the constraint. A better generator must be given then.
         */
        virtual std::unique_ptr<DataValueGenerator> makeGenerator() const {
            throw std::runtime_error("Dimensions::Constraint::makeGenerator: no generator can be made from: " + this->toString());
        }

    };

    /**
     * \brief Default Unconstrained data
     */
    struct UnconstrainedData final : Constraint {
        /// \brief Accepts every view.
        bool accepts(const Data::DataView&) const override {
            return true;
        }

        /**
         * \brief Return true if the consumer is compatible with the current constraint.
         * 
         * For Unconstrained data, return true only if the consumer is unconstrained too.
         * 
         * \param[in] consumer the constraint checked
         */
        bool isCompatibleWith(const Constraint& consumer) const override {
            return dynamic_cast<const UnconstrainedData*>(&consumer) != nullptr;
        }


        /** \brief Compares this unconstrained marker with another constraint.
         * \param[in] other Constraint to compare with.
         * \return `true` only when the other constraint is also unconstrained.
         */
        bool equals(const Constraint& other) const override {
            return dynamic_cast<const UnconstrainedData*>(&other) != nullptr;
        }

        /// \brief Creates a shared copy of the unconstrained marker.
        std::shared_ptr<const Constraint> cloneSharedPtr() const override {
            return std::make_shared<const UnconstrainedData>(*this);
        }

        /// \brief Returns the unconstrained diagnostic label.
        std::string toString() const override {
            return "unconstrained";
        }
    };
    /**
     * \brief operator for printing
     */
    inline std::ostream& operator<<(std::ostream& os, const Constraint& constraint) {
        return os << constraint.toString();
    }

    
    /**
     * \brief Return true if both constraints are equal (see Constraint::equals).
     */
    inline bool operator==(const Constraint& a, const Constraint& b) {
        return a.equals(b);
    }

    /**
     * \brief Return true if the constraints differ (see Constraint::equals).
     */
    inline bool operator!=(const Constraint& a, const Constraint& b){
        return !(a == b);
    }
};

#endif