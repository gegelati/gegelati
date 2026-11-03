#ifndef EVOLUTION_KEY_H
#define EVOLUTION_KEY_H

#include <ostream>
#include <string>
#include <utility>

namespace Evolution {

    /**
     * \brief Key of a data of the EvolutionData, tagged with the type of the data.
     *
     * A key is a name with a compile-time type: it belongs to no population, and several steps (or several
     * populations) can use the same key or different ones as they wish. The type makes a step refuse a key of
     * the wrong type when it is written, instead of when it runs. The state is only accessed through keys: the
     * name only identifies the data, and is printed in summaries.
     *
     * \tparam T type of the data (the state stores a std::shared_ptr<T>).
     */
    template <typename T> class Key {
      public:
        /// Type of the data.
        using Type = T;

        /// Create a key from the name of the data.
        explicit Key(std::string name) : keyName{std::move(name)} {}

        /// Name of the data.
        const std::string& name() const { return this->keyName; }

        /// Two keys are equal if they have the same name.
        bool operator==(const Key& other) const { return this->keyName == other.keyName; }

        /// Two keys are different if they have different names.
        bool operator!=(const Key& other) const { return this->keyName != other.keyName; }

      private:
        /// Name of the data.
        std::string keyName;
    };

    /// Print the name of the key (e.g. in test failures).
    template <typename T> std::ostream& operator<<(std::ostream& stream, const Key<T>& key)
    {
        return stream << key.name();
    }

}; // namespace Evolution

#endif // EVOLUTION_KEY_H
