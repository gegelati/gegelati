#ifndef EVOLUTION_ENTRY_H
#define EVOLUTION_ENTRY_H

#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>

namespace Evolution {

    /**
     * \brief Data stored in an EvolutionData, whatever its type.
     *
     * It knows the generation in which the data was stored, the type of the data, and how to describe it in summaries.
     * The data itself is in Entry<T>, which EvolutionData gets back through the Key<T> used to store it.
     */
    class EntryBase {
      public:
        /// Constructor.
        explicit EntryBase(uint64_t generation) : storedGeneration{generation} {}

        /// Default polymorphic destructor.
        virtual ~EntryBase() = default;

        /// Generation in which the data was stored.
        uint64_t generation() const { return this->storedGeneration; }

        /// Return the type of the data, T for an Entry<T>.
        virtual std::type_index type() const = 0;

        /// Describe the current content of the data (its size, or its value for a number) for summaries.
        virtual std::string describe() const = 0;

      private:
        /// Generation in which the data was stored.
        uint64_t storedGeneration;
    };

    /**
     * \brief Data of type T stored in an EvolutionData.
     *
     * The entry is immutable once stored: storing a new data replaces the entry. The data it points to can be modified.
     *
     * \tparam T type of the data (the entry holds a std::shared_ptr<T>).
     */
    template <typename T> class Entry final : public EntryBase {
      public:
        /**
         * \brief Constructor.
         *
         * \param[in] value the data, shared with the caller.
         * \param[in] generation generation in which the data is stored.
         */
        Entry(std::shared_ptr<T> value, uint64_t generation) : EntryBase(generation), data{std::move(value)} {}

        /// The data.
        const std::shared_ptr<T>& value() const { return this->data; }

        /// Inherited from EntryBase.
        std::type_index type() const override { return typeid(T); }

        /// Inherited from EntryBase.
        std::string describe() const override { return (this->data == nullptr) ? std::string("null") : describeValue(*this->data, 0); }

      private:
        /// The data.
        std::shared_ptr<T> data;

        /// Describe a value that has a size, e.g. a list or a map.
        template <typename U> static auto describeValue(const U& value, int) -> decltype(value.size(), std::string())
        {
            return std::to_string(value.size()) + " elements";
        }

        /// Describe any other value: its content if it is a number, nothing otherwise.
        template <typename U> static std::string describeValue(const U& value, long)
        {
            if constexpr(std::is_arithmetic_v<U>) {
                return std::to_string(value);
            } else {
                (void)value;
                return "";
            }
        }
    };

}; // namespace Evolution

#endif // EVOLUTION_ENTRY_H
