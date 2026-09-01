#ifndef R_DATA_VALUE_H
#define R_DATA_VALUE_H

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <typeinfo>
#include <utility>

#include "data/dataShape.h"

namespace Data {

    /**
    * Non-owning, read-only view passed to an instruction.
     *
     * DataViewOld is the input-side half of the instruction data boundary. It
     * contains only a pointer, runtime type information, and a DataShape. It
    * never deletes or copies the pointed-to object. The caller must keep the
    * source object alive and unchanged for as long as the view is used.
     *
    * Use scalar() for a single T and array() for contiguous storage. The
    * typed accessors validate the stored type before returning a reference or
    * pointer. A null view or a type mismatch throws std::runtime_error.
    *
    * DataViewOld is cheap to copy because it copies only a pointer and metadata.
    * Copying a view never extends the lifetime of its source.
     */
    class DataViewOld
    {
      public:
            /** Construct an invalid view with no storage. */
        DataViewOld() = default;

        /**
         * Create a view of one scalar without copying it.
         *
         * The referenced object must outlive every use of the returned view.
         * Its recorded type and shape are typeid(T) and {1}.
         */
        template <typename T> static DataViewOld scalar(const T& value)
        {
            return DataViewOld{&value, typeid(T), DataShape{1}};
        }

        /**
         * Create a view of contiguous array storage without copying it.
         *
         * Array extents are stored in the supplied DataShape. The pointer is
         * treated as flat contiguous storage, including for rank-2 shapes.
         */
        template <typename T>
        static DataViewOld array(const T* values, DataShape shape)
        {
            return DataViewOld{values, typeid(T[]), std::move(shape)};
        }

        /** Return the raw borrowed pointer. */
        const void* data() const noexcept { return pointer; }

        /** Return the runtime type recorded when the view was created. */
        const std::type_info& type() const noexcept { return *typeInfo; }

        /** Return the logical shape recorded when the view was created. */
        const DataShape& shape() const noexcept { return valueShape; }

        /** Return a checked reference to a scalar T. */
        template <typename T> const T& getScalar() const
        {
            check(typeid(T), DataShape{1});
            return *static_cast<const T*>(pointer);
        }

        /** Return a checked pointer to contiguous array elements of type T. */
        template <typename T> const T* getArray() const
        {
            if (pointer == nullptr || *typeInfo != typeid(T[])) {
                throw std::runtime_error("DataViewOld type mismatch.");
            }
            return static_cast<const T*>(pointer);
        }
        

      private:
            /** Only DataValueOld can create a view of its owned storage directly. */
        friend class DataValueOld;

            /** Store borrowed storage and its runtime metadata. */
        DataViewOld(const void* pointer, const std::type_info& typeInfo,
                 DataShape shape)
            : pointer(pointer), typeInfo(&typeInfo),
              valueShape(std::move(shape))
        {
        }

        /** Validate storage, type, and shape before typed access. */
        void check(const std::type_info& expected,
                   const DataShape& expectedShape) const
        {
            if (pointer == nullptr || *typeInfo != expected ||
                valueShape != expectedShape) {
                throw std::runtime_error("DataViewOld type or shape mismatch.");
            }
        }

        /// Borrowed address of the first byte of the viewed value.
        const void* pointer = nullptr;

        /// Runtime type recorded for the viewed scalar or array elements.
        const std::type_info* typeInfo = &typeid(void);

        /// Logical interpretation of the borrowed storage.
        DataShape valueShape;
    };

    /**
     * Owning, type-erased result produced by an instruction.
     *
     * DataValueOld is the output-side half of the instruction data boundary. It
     * owns either one scalar object or a dynamically allocated contiguous
     * array. Copies perform deep copies; moves transfer ownership. The class
     * stores runtime type information and a DataShape so a runtime dispatcher
     * can validate a result without knowing its C++ type at compile time.
     *
     * DataValueOld is intentionally separate from DataViewOld: an instruction may
     * borrow its inputs, but its result must remain valid after the input
     * objects or the instruction call have gone out of scope.
     *
     * The class uses type erasure internally: Concept defines the operations
     * required by the erased storage, while ScalarModel<T> and ArrayModel<T>
     * provide the concrete storage for each T. Users normally do not need to
     * interact with those implementation types.
     *
     * Scalar values report typeid(T) and shape {1}. Array values report
     * typeid(T[]) and currently use a rank-1 shape containing their element
     * count. A rank-2 shape can be carried by DataViewOld, but this DataValueOld
     * factory currently creates only rank-1 array results.
     */
    class DataValueOld
    {
        struct Concept
        {
            /// Enable destruction through the erased base type.
            virtual ~Concept() = default;

            /// Return the address of the owned value's storage.
            virtual const void* data() const noexcept = 0;

            /// Create an independent deep copy of the concrete model.
            virtual std::unique_ptr<Concept> clone() const = 0;
        };

        template <typename T> struct ScalarModel final : Concept
        {
            /// Construct the concrete scalar storage.
            explicit ScalarModel(T value) : value(std::move(value)) {}

            /// Return the address of the scalar object.
            const void* data() const noexcept override { return &value; }

            /// Copy the scalar into another erased model.
            std::unique_ptr<Concept> clone() const override
            {
                return std::unique_ptr<Concept>(new ScalarModel<T>(value));
            }

            /// The actual owned scalar.
            T value;
        };

        template <typename T> struct ArrayModel final : Concept
        {
            /** Take ownership of `values` and remember its logical length. */
            ArrayModel(std::unique_ptr<T[]> values, size_t count)
                : values(std::move(values)), count(count)
            {
            }

            /// Return the address of the first element of the owned array.
            const void* data() const noexcept override { return values.get(); }

            /// Deep-copy the array and its element count.
            std::unique_ptr<Concept> clone() const override
            {
                return std::unique_ptr<Concept>(
                    new ArrayModel<T>(copyValues(values.get()), count));
            }

            /// Allocate and copy the array used by a cloned model.
            std::unique_ptr<T[]> copyValues(const T* source) const
            {
                std::unique_ptr<T[]> copy(new T[count]);
                std::copy(source, source + count, copy.get());
                return copy;
            }

            /// Contiguous array storage owned by this model.
            std::unique_ptr<T[]> values;

            /// Number of elements; unique_ptr<T[]> does not carry a length.
            size_t count;
        };

        template <typename T> struct Array2dModel final : Concept
        {
            /** Take ownership of `values` and remember its logical dimensions. */
            Array2dModel(std::unique_ptr<T[]> values, size_t rows, size_t cols)
                : values(std::move(values)), rows(rows), cols(cols)
            {
            }

            /// Return the address of the first element of the owned array.
            const void* data() const noexcept override { return values.get(); }

            /// Deep-copy the array and its dimensions.
            std::unique_ptr<Concept> clone() const override
            {
                return std::unique_ptr<Concept>(
                    new Array2dModel<T>(copyValues(values.get()), rows, cols));
            }

            /// Allocate and copy the array used by a cloned model.
            std::unique_ptr<T[]> copyValues(const T* source) const
            {
                std::unique_ptr<T[]> copy(new T[rows * cols]);
                std::copy(source, source + rows * cols, copy.get());
                return copy;
            }

            /// Contiguous array storage owned by this model.
            std::unique_ptr<T[]> values;

            /// Number of rows.
            size_t rows;

            /// Number of columns.
            size_t cols;
        };

      public:
        /// A result must contain a scalar or array, so an empty value is not allowed.
        DataValueOld() = delete;

        /** Create an owning scalar result by moving or copying `value`. */
        template <typename T> static DataValueOld scalar(T value)
        {
            return DataValueOld(
                std::unique_ptr<Concept>(new ScalarModel<T>(std::move(value))),
                typeid(T), DataShape{1});
        }

        /**
         * Create an owning contiguous 1D array result.
         *
         * The input pointer becomes owned by the returned DataValueOld and must
         * point to at least `count` elements. The caller must not delete it
         * afterwards. `count` is used for shape reporting and deep copies.
         */
        template <typename T>
        static DataValueOld array(std::unique_ptr<T[]> values, size_t count)
        {
            return DataValueOld(
                std::unique_ptr<Concept>(
                    new ArrayModel<T>(std::move(values), count)),
                typeid(T[]), DataShape{count});
        }

        /**
         * Create an owning contiguous 2D array result.
         *
         * The input pointer becomes owned by the returned DataValueOld and must
         * point to at least `rows * cols` elements. The caller must not delete it
         * afterwards. `rows` and `cols` are used for shape reporting and deep copies.
         */
        template <typename T>
        static DataValueOld array2d(std::unique_ptr<T[]> values, size_t rows, size_t cols)
        {
            return DataValueOld(
                std::unique_ptr<Concept>(
                    new Array2dModel<T>(std::move(values), rows, cols)),
                typeid(T[]), DataShape{rows, cols});
        }

        /** Deep-copy an owned result. */
        DataValueOld(const DataValueOld& other)
            : storage(other.storage->clone()), typeInfo(other.typeInfo),
              valueShape(other.valueShape)
        {
        }

        /** Transfer ownership without copying the stored object. */
        DataValueOld(DataValueOld&&) noexcept = default;
        /** Deep-copy assignment of an owned result. */
        DataValueOld& operator=(const DataValueOld& other)
        {
            if (this != &other) {
                storage = other.storage->clone();
                typeInfo = other.typeInfo;
                valueShape = other.valueShape;
            }
            return *this;
        }
        /** Move assignment transfers the owned storage. */
        DataValueOld& operator=(DataValueOld&&) noexcept = default;

        /** Return the runtime type of the stored scalar or array elements. */
        const std::type_info& type() const noexcept { return *typeInfo; }

        /** Return the logical shape of the stored result. */
        const DataShape& shape() const noexcept { return valueShape; }

        /** Borrow the owned result as a read-only view. */
        DataViewOld view() const noexcept
        {
            return DataViewOld{storage->data(), *typeInfo, valueShape};
        }

        /** Return a checked reference to an owned scalar T. */
        template <typename T> const T& getScalar() const
        {
            return view().getScalar<T>();
        }

        /** Return a checked pointer to owned contiguous array elements. */
        template <typename T> const T* getArray() const
        {
            return view().getArray<T>();
        }


      private:
            /** Construct a result from erased storage and its metadata. */
        DataValueOld(std::unique_ptr<Concept> storage,
                  const std::type_info& typeInfo, DataShape shape)
            : storage(std::move(storage)), typeInfo(&typeInfo),
              valueShape(std::move(shape))
        {
        }

        /// Concrete scalar or array model owning the result memory.
        std::unique_ptr<Concept> storage;

        /// Runtime type recorded for the owned value.
        const std::type_info* typeInfo;

        /// Logical interpretation of the owned storage.
        DataShape valueShape;
    };

} // namespace Data

#endif