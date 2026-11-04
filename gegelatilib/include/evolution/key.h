#ifndef EVOLUTION_KEY_H
#define EVOLUTION_KEY_H

#include <ostream>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <utility>

namespace Evolution {

    /**
     * \brief Key of a data without its compile-time type: a name and the runtime type of the data.
     *
     * It is what modules declare as required or provided data, whatever the type of the data, so the types can
     * be checked before an evolution runs (see Module::checkData). A key built from a name only is untyped: it is
     * compatible with any type, e.g. for the data of a user function that does not know the types.
     */
    class KeyBase {
      public:
        /// Create an untyped key from the name of the data.
        KeyBase(std::string name) : keyName{std::move(name)}, keyType{typeid(void)} {}

        /// Name of the data.
        const std::string& name() const { return this->keyName; }

        /// Runtime type of the data, typeid(void) if the key is untyped.
        std::type_index type() const { return this->keyType; }

        /// Return true if the key has a type.
        bool isTyped() const { return this->keyType != std::type_index(typeid(void)); }

        /// Return true if the keys have no different types (an untyped key is compatible with any type).
        bool compatibleWith(const KeyBase& other) const { return !this->isTyped() || !other.isTyped() || this->keyType == other.keyType; }

      protected:
        /// Create a key from the name and type of the data.
        KeyBase(std::string name, std::type_index type) : keyName{std::move(name)}, keyType{type} {}

      private:
        /// Name of the data.
        std::string keyName;

        /// Runtime type of the data.
        std::type_index keyType;
    };

    /**
     * \brief Key of a data of the EvolutionData, tagged with the type of the data.
     *
     * A key is a name with a compile-time type: it belongs to no population, and several steps (or several
     * populations) can use the same key or different ones as they wish. The type makes a step refuse a key of
     * the wrong type when it is written, instead of when it runs. The state is only accessed through keys: the
     * name only identifies the data, and is printed in summaries. A key converts to a KeyBase to be declared
     * as required or provided by a module.
     *
     * \tparam T type of the data (the state stores a std::shared_ptr<T>).
     */
    template <typename T> class Key : public KeyBase {
      public:
        /// Type of the data.
        using Type = T;

        /// Create a key from the name of the data.
        explicit Key(std::string name) : KeyBase(std::move(name), typeid(T)) {}

        /// Two keys are equal if they have the same name.
        bool operator==(const Key& other) const { return this->name() == other.name(); }

        /// Two keys are different if they have different names.
        bool operator!=(const Key& other) const { return this->name() != other.name(); }
    };

    /// Print the name of the key (e.g. in test failures).
    template <typename T> std::ostream& operator<<(std::ostream& stream, const Key<T>& key)
    {
        return stream << key.name();
    }

}; // namespace Evolution

#endif // EVOLUTION_KEY_H
