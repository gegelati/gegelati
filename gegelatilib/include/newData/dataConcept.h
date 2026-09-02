#ifndef DATA_CONCEPT_H
#define DATA_CONCEPT_H

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>

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
             * \brief Returns a pointer to the raw stored data.
             *
             * \return Pointer to the underlying value storage in a type-erased form.
             */
            virtual const void* data() const noexcept = 0;

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
             * \param v Value stored by the model.
             */
            explicit ScalarModel(T v) : value(std::move(v)) {}

            /**
             * \brief Returns the address of the scalar value.
             *
             * \return Pointer to the scalar storage.
             */
            const void* data() const noexcept override { return &value; }

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
        };

        /// \brief Storage model for a contiguous 1D array.
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
             * \brief Constructs a 1D array storage model.
             *
             * \param v Buffer owning the array elements.
             * \param c Number of elements in the buffer.
             */
            ArrayModel(std::unique_ptr<T[]> v, size_t c) : values(std::move(v)), count(c) {}

            /**
             * \brief Returns the address of the array buffer.
             *
             * \return Pointer to the first element of the array.
             */
            const void* data() const noexcept override { return values.get(); }

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
                return std::make_unique<ArrayModel<T>>(std::move(copy), count);
            }

            /**
             * \brief Serializes the array as a bracketed list of values.
             *
             * \return String representation of the 1D array.
             */
            std::string toString() const override {
                std::ostringstream oss;
                oss << "[";

                for (size_t i = 0; i < count; ++i) {
                    if (i > 0) {
                        oss << ", ";
                    }
                    oss << values[i];
                }

                oss << "]";
                return oss.str();
            }
        };

        /// \brief Storage model for a row-major 2D array.
        template <typename T>
        struct Array2dModel final : ValueConcept {
            /**
             * \brief Dynamic buffer storing the row-major matrix values.
             */
            std::unique_ptr<T[]> values;

            /**
             * \brief Number of rows in the 2D array.
             */
            size_t rows;

            /**
             * \brief Number of columns in the 2D array.
             */
            size_t cols;

            /**
             * \brief Constructs a 2D array storage model.
             *
             * \param v Buffer owning the row-major array data.
             * \param r Number of rows.
             * \param c Number of columns.
             */
            Array2dModel(std::unique_ptr<T[]> v, size_t r, size_t c)
                : values(std::move(v)), rows(r), cols(c) {}

            /**
             * \brief Returns the address of the 2D array buffer.
             *
             * \return Pointer to the first element of the row-major storage.
             */
            const void* data() const noexcept override { return values.get(); }

            /**
             * \brief Returns the mutable address of the 2D array buffer.
             *
             * \return Pointer to the first element of the row-major storage.
             */
            void* data() noexcept override { return values.get(); }

            /**
             * \brief Creates a deep copy of the 2D array model.
             *
             * \return A deep clone containing the same row-major values.
             */
            std::unique_ptr<ValueConcept> clone() const override {
                const size_t total = rows * cols;
                auto copy = std::make_unique<T[]>(total);
                std::copy(values.get(), values.get() + total, copy.get());
                return std::make_unique<Array2dModel<T>>(std::move(copy), rows, cols);
            }

            /**
             * \brief Serializes the matrix as a nested bracketed list.
             *
             * \return String representation of the 2D array.
             */
            std::string toString() const override {
                std::ostringstream oss;
                oss << "[";

                for (size_t row = 0; row < rows; ++row) {
                    if (row > 0) {
                        oss << ", ";
                    }

                    oss << "[";
                    for (size_t col = 0; col < cols; ++col) {
                        if (col > 0) {
                            oss << ", ";
                        }
                        oss << values[row * cols + col];
                    }
                    oss << "]";
                }

                oss << "]";
                return oss.str();
            }
        };

    } // namespace detail

} // namespace Data

#endif // DATA_CONCEPT_H
