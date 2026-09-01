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
         * Each concrete model owns the underlying data buffer for one logical value:
         * scalar, 1D array, or 2D array. This keeps the public DataValue API small
         * while preserving ownership, cloning, and debug rendering.
         */
        struct ValueConcept {
            virtual ~ValueConcept() = default;

            virtual const void* data() const noexcept = 0;
            virtual void* data() noexcept = 0;
            virtual std::unique_ptr<ValueConcept> clone() const = 0;
            virtual std::string toString() const = 0;
        };

        /// \brief Storage model for a scalar value.
        template <typename T>
        struct ScalarModel final : ValueConcept {
            T value;

            explicit ScalarModel(T v) : value(std::move(v)) {}

            const void* data() const noexcept override { return &value; }
            void* data() noexcept override { return &value; }

            std::unique_ptr<ValueConcept> clone() const override {
                return std::make_unique<ScalarModel<T>>(value);
            }

            std::string toString() const override {
                std::ostringstream oss;
                oss << value;
                return oss.str();
            }
        };

        /// \brief Storage model for a contiguous 1D array.
        template <typename T>
        struct ArrayModel final : ValueConcept {
            std::unique_ptr<T[]> values;
            size_t count;

            ArrayModel(std::unique_ptr<T[]> v, size_t c) : values(std::move(v)), count(c) {}

            const void* data() const noexcept override { return values.get(); }
            void* data() noexcept override { return values.get(); }

            std::unique_ptr<ValueConcept> clone() const override {
                auto copy = std::make_unique<T[]>(count);
                std::copy(values.get(), values.get() + count, copy.get());
                return std::make_unique<ArrayModel<T>>(std::move(copy), count);
            }

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
            std::unique_ptr<T[]> values;
            size_t rows;
            size_t cols;

            Array2dModel(std::unique_ptr<T[]> v, size_t r, size_t c)
                : values(std::move(v)), rows(r), cols(c) {}

            const void* data() const noexcept override { return values.get(); }
            void* data() noexcept override { return values.get(); }

            std::unique_ptr<ValueConcept> clone() const override {
                const size_t total = rows * cols;
                auto copy = std::make_unique<T[]>(total);
                std::copy(values.get(), values.get() + total, copy.get());
                return std::make_unique<Array2dModel<T>>(std::move(copy), rows, cols);
            }

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
