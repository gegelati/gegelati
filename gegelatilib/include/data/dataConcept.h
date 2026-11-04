#ifndef DATA_CONCEPT_H
#define DATA_CONCEPT_H

#include <algorithm>
#include <cstddef>
#include <memory>
#include <sstream>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "data/dataType.h"

namespace Data {

    namespace detail {

        /**
         * \brief Type-erased storage contract used by DataValue.
         *
         * Each concrete implementation owns the memory backing a single logical
         * value, whether it is a scalar, a 1D array, or a 2D array. This keeps the
         * public DataValue interface small while preserving ownership, deep-copying,
         * and debug-friendly string rendering.
         *
         * \note The concept provides virtual access to the raw storage buffer as
         * well as polymorphic cloning and formatting helpers for introspection.
         */
        struct ValueConcept {
            /**
             * \brief Destroys the underlying storage model.
             *
             * The default implementation ensures correct cleanup of any concrete
             * storage implementation through polymorphic deletion.
             */
            virtual ~ValueConcept() = default;

            /**
             * \brief Returns a mutable pointer to the raw stored data.
             *
             * \return Mutable pointer to the underlying value storage.
             */
            virtual void* data() noexcept = 0;

            /**
             * \brief Creates a deep copy of the current storage model.
             *
             * \return A newly allocated clone of the concrete storage instance.
             */
            virtual std::unique_ptr<ValueConcept> clone() const = 0;

            /**
             * \brief Serializes the stored value as a human-readable string.
             *
             * \return A debug-friendly representation of the contained value.
             */
            virtual std::string toString() const = 0;
            
            /**
             * \brief Compares this value with another type-erased value.
             *
             * \return true if both values have the same concrete model and
             *         represent equal values.
             */
            virtual bool equals(const ValueConcept& other) const = 0;
        };

        /// \brief Storage model for a scalar value.
        template <typename T>
        struct ScalarModel final : ValueConcept {
            /**
             * \brief Stored scalar value.
             */
            T value;

            /**
             * \brief Constructs a scalar storage model.
             *
             * \param[in] v Value stored by the model.
             */
            explicit ScalarModel(T v) : value(std::move(v)) {}

            /**
             * \brief Returns the mutable address of the scalar value.
             *
             * \return Pointer to the scalar storage for modification.
             */
            void* data() noexcept override { return &value; }

            /**
             * \brief Creates a copy of the scalar model.
             *
             * \return A deep clone of the scalar storage.
             */
            std::unique_ptr<ValueConcept> clone() const override {
                return std::make_unique<ScalarModel<T>>(value);
            }

            /**
             * \brief Converts the scalar value to a string.
             *
             * \return String representation of the scalar value.
             */
            std::string toString() const override {
                std::ostringstream oss;
                oss << value;
                return oss.str();
            }

            /**
             * \brief scalar equality check
             */
            bool equals(const ValueConcept& other) const override {
                const auto* otherPtr = dynamic_cast<const ScalarModel<T>*>(&other);
    
                if (otherPtr == nullptr) { return false; }

                return this->value == otherPtr->value;
            }
        };

        /// \brief Storage model for a contiguous row-major N-dimensional array.
        template <typename T>
        struct ArrayModel final : ValueConcept {
            /**
             * \brief Dynamic buffer containing the array elements.
             */
            std::unique_ptr<T[]> values;

            /**
             * \brief Number of elements stored in the array.
             */
            size_t count;

            /**
             * \brief Extent of each axis (its size is the rank).
             */
            Extents shape;

            /**
             * \brief Constructs an N-dimensional array storage model.
             *
             * \param[in] v Buffer owning the array elements.
             * \param[in] s Extent of each axis; the buffer holds their product.
             */
            ArrayModel(std::unique_ptr<T[]> v, Extents s)
                : values(std::move(v)), count(1), shape(std::move(s)) {
                for (size_t extent : shape) {
                    count *= extent;
                }
            }

            /**
             * \brief Returns the mutable address of the array buffer.
             *
             * \return Pointer to the first element of the array for modification.
             */
            void* data() noexcept override { return values.get(); }

            /**
             * \brief Creates a deep copy of the array model.
             *
             * \return A deep clone containing the same array values.
             */
            std::unique_ptr<ValueConcept> clone() const override {
                auto copy = std::make_unique<T[]>(count);
                std::copy(values.get(), values.get() + count, copy.get());
                return std::make_unique<ArrayModel<T>>(std::move(copy), shape);
            }

            /**
             * \brief Serializes the array as nested bracketed lists.
             *
             * \return String representation of the array.
             */
            std::string toString() const override {
                std::ostringstream oss;
                size_t index = 0;
                write(oss, 0, index);
                return oss.str();
            }

            /**
             * \brief array equality check (shape and values)
             */
            bool equals(const ValueConcept& other) const override {
                const auto* otherPtr = dynamic_cast<const ArrayModel<T>*>(&other);

                if (otherPtr == nullptr || this->shape != otherPtr->shape) {
                    return false;
                }

                return std::equal(this->values.get(), this->values.get() + count, otherPtr->values.get());
            }

        private:
            /// \brief Recursively prints the axis `axis` starting at the linear `index`.
            void write(std::ostringstream& oss, size_t axis, size_t& index) const {
                oss << "[";
                for (size_t i = 0; i < shape[axis]; ++i) {
                    if (i > 0) {
                        oss << ", ";
                    }
                    if (axis + 1 == shape.size()) {
                        oss << values[index++];
                    } else {
                        write(oss, axis + 1, index);
                    }
                }
                oss << "]";
            }
        };

    } // namespace detail

} // namespace Data

#endif // DATA_CONCEPT_H
