#ifndef UTIL_NAMED_H
#define UTIL_NAMED_H

#include <ostream>
#include <string>
#include <utility>

namespace Util {

    /**
     * \brief Base class of the objects that have a name to be printed, like the layers of a deep learning framework.
     *
     * The name is only for printing (summary of an evolution, logs, error messages): two objects of the same class, e.g. two
     * tournament selectors, can be given different names to tell them apart. It is not an identifier.
     *
     * getDetails describes the object with its parameters. By default it is the name, and the classes that have parameters
     * override it, usually as name(parameters), e.g. "Small(size 3, with replacement)". Writing an object in a stream writes its name.
     */
    class Named {
      private:
        /// Name of the object.
        std::string name;

      protected:
        /**
         * \brief Constructor.
         *
         * \param[in] name the name of the object, only used to print it.
         */
        explicit Named(std::string name) : name{std::move(name)} {}

        /// Copy, made by the derived classes.
        Named(const Named&) = default;

        /// Copy, made by the derived classes.
        Named& operator=(const Named&) = default;

        /// Not public: an object is never destroyed through a pointer to Named, the derived classes have their own virtual destructor.
        ~Named() = default;

      public:
        /// Return the name of the object.
        const std::string& getName() const { return this->name; }

        /// Return the description of the object with its parameters, the name by default.
        virtual std::string getDetails() const { return this->name; }
    };

    /// Write the name of an object in a stream.
    inline std::ostream& operator<<(std::ostream& os, const Named& named)
    {
        return os << named.getName();
    }

}; // namespace Util

#endif // UTIL_NAMED_H
