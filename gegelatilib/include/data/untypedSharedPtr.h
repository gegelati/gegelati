/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#ifndef UNTYPED_SHARED_PTR_H
#define UNTYPED_SHARED_PTR_H

#include <functional>
#include <memory>
#include <type_traits>

namespace Data {

    /**
     * \brief Class behaving as a std::shared_ptr whose type is not templated.
     *
     * Instances of this class is that it behaves as a share_ptr, meaning that
     * it contains a pointer to an object that is freed automatically when the
     * last copy of the UntypedSharedPtr associated to this pointer is deleted.
     * The deleter that can be passed as a parameter when building an instance
     * is used to delete the object. By giving an empty function to the
     * constructor, the deletion of the pointer can thus be prevented, and the
     * UntypedSharedPtr can be used to store a regular pointer.
     *
     * The main difference with the classical std::shared_ptr<T> is that no
     * template parameter specifies what is stored inside the UntypedSharedPtr.
     * Hence, a std:vector<UntypedSharedPtr> can contain shared pointer to any
     * type of data (including primitive types), whereas a std::vector<
     * std::shared_ptr<Base>> can contain only pointers to types derived from
     * a given Base class, which notably prevent such a vector from containing
     * shared pointer to several distinct primitive types.
     *
     * The code of this class is based on the Type Erasure patterns, and is
     * directly inspired by [this
     * example](https://www.modernescpp.com/index.php/c-core-guidelines-type-erasure-with-templates)/
     */
    class UntypedSharedPtr
    {
      public:
        /**
         * \brief Internal structure of the type erasure pattern.
         */
        struct Concept
        {
            /// Default deleter made virtual to activate polyphormism.
            virtual ~Concept() = default;
            /// Polymorphic getType() function.
            virtual const std::type_info& getType() const = 0;
            /// Polymorphic getPtrType() function.
            virtual const std::type_info& getPtrType() const = 0;
        };

        /**
         * \brief Internal templated structure of the type erasure pattern.
         *
         * This part of the type erasure pattern actually contains the
         * std::shared_ptr.
         *
         * \tparam T Template type taken from UntypedSharedPtr constructor.
         */
        template <typename T> struct Model : Concept
        {
          public:
            /// Constructor of the Model: initializes the internal
            /// std::shared_ptr.
            template <typename Deleter>
            Model(T* t, Deleter func) : sharedPtr(t, func)
            {
            }

            /// Constructor for model for array
            /// (whose type decay into pointers when passed as arguments.)
            /// Default deleter automatically used
            template <typename U, typename _ = typename std::enable_if<
                                      std::is_array<T>::value, U>::type>
            Model(U* p) : sharedPtr(p, std::default_delete<T>()){};

            /// Polymorphic getType() function.
            const std::type_info& getType() const override
            {
                return typeid(T);
            }

            /// Polymorphic getPtrType() function.
            const std::type_info& getPtrType() const override
            {
                return typeid(sharedPtr.get());
            }

            /// std::shared_ptr of the UntypedSharedPtr
            std::shared_ptr<T> sharedPtr;
        };

        /**
         * \brief Deleted default constructor.
         */
        UntypedSharedPtr() = delete;

        /**
         * \brief Main constructor of the UntypedSharedPtr class.
         *
         * Constructs an instance of the UntypedSharedPtr class whose type is
         * given as a template parameter T that is deduced at function call.
         *
         * For example, the following codes creates an UntypedSharedPtr
         * for an int value:
         * \code{.cpp}
         * UntypedSharedPtr ptr{new int(2)}; // Template parameter 'int' deduced
         * \endcode
         *
         * The default deleter for the given pointer type is used by default,
         * unless a deleter is explicitly given to the constructor.
         * For example, a deleter doing nothing can be given to prevent the
         * shared pointer from deallocating the memory of a variable on the
         * stack.
         * \code{.cpp}
         * // Empty Deleter for double pointers.
         * auto del = [](const double *){};
         *
         * // Variable on the heap
         * double a{2.5};
         *
         * { // Beginning of a scope
         *   UntypedSharedPtr ptrA{&a, del};
         *   UntypedSharedPtr ptr{new int(2)};
         *   // UntypedSharedPtr ptrA2{&a}; // Problem => deletion of the memory
         *                                  // will be attempted when leaving
         *                                  // ptrA2 disappears.
         * } // End of a scope
         *   // Memory associated to ptr is freed with default deleter for int*
         *   // Memory associated to ptrA is deleted with del, which does
         * nothing. \endcode As it is the case with std::shared_ptr, when
         * misused, these capacity may result in invalid data access.
         *
         * \tparam T type of the equivalent std::shared_pointer<T>. Beware,
         * const qualifier matters.
         * \tparam Deleter type of the deletion function. Default value is
         * std::default_delete<T>
         * \param[in] obj Pointer to the memory managed by the UntypedSharedPtr.
         * \param[in] func The function used as Deleter when the last copy of
         * the UntypedSharedPtr disappears. Default value is
         * std::default_delete<T>().
         */
        template <typename T, class Deleter = std::default_delete<T>>
        UntypedSharedPtr(T* obj, Deleter func = Deleter())
            : sharedPtrContainer(std::make_shared<Model<T>>(obj, func)){};

        /**
         * \brief Constructor from an existing Concept.
         *
         * This constructor is needed when the pointer type passed to the
         * classical constructor decays automatically into something else. For
         * example, any pointer to a C-style array allocated with 'new T[]'
         * automatically decays to 'T*' when passed to a function. By
         * constructing a Model<W> with the desired template type (eg.
         * W = T[]), it is possible to force the 'getPtr()' method to return
         * this 'T[]' type.
         *
         * \param[in] concept the instance of the Model<T> class for building
         * the UntypedSharedPointer.
         */
        UntypedSharedPtr(std::shared_ptr<Concept> concept)
            : sharedPtrContainer(concept){};

        /**
         * \brief Accessor to the type of data stored in the UntypedSharedPtr.
         *
         * \return a const ref to the std::type_info of the template type T of
         * UntypedSharedPtr at construction, or the Model template type for the
         * constructor based on std::shared_ptr<Concept>. Please note that this
         * type may differ from the actual type of the data, if this data has
         * a type derived from the base pointer type T. Also note that any
         * const qualifier will be lost.
         *
         * \code{.cpp}
         * class Base {};
         * class Derived : public Base {};
         *
         * // Non const example.
         * Base * ptrA = new Derived();
         * UntypedSharedPtr uspA{ptrA}; // shared pointer type is Base.
         * std::cout << std::uspA.getType().name();  // Prints "Base"
         *
         * // Const example
         * const Base * ptrB = new Derived();
         * UntypedSharedPtr uspB{ptrA}; // shared pointer type is const Base.
         * std::cout << std::uspB.getType().name();  // Prints "Base"
         * \endcode
         */
        const std::type_info& getType() const
        {
            return sharedPtrContainer->getType();
        }

        /**
         * \brief Accessor to the pointer type of data stored in the
         * UntypedSharedPtr.
         *
         * \return a const ref to the std::type_info of the pointer type T* of
         * UntypedSharedPtr at construction. Please note that this may differ
         * from the actual type of the data, if this data has a type derived
         * from the base pointer type T. Contrary to the getType method, const
         * qualifiers are preserved with pointer types.
         *
         * \code{.cpp}
         * class Base {};
         * class Derived : public Base {};
         *
         * // Non const example.
         * Base * ptrA = new Derived();
         * UntypedSharedPtr uspA{ptrA}; // shared pointer type is Base.
         * std::cout << std::uspA.getPtrType().name();  // Prints "Base *"
         *
         * // Const example
         * const Base * ptrB = new Derived();
         * UntypedSharedPtr uspB{ptrA}; // shared pointer type is const Base.
         * std::cout << std::uspB.getPtrType().name();  // Prints "const Base*"
         * \endcode
         */
        const std::type_info& getPtrType() const
        {
            return sharedPtrContainer->getPtrType();
        }

        /**
         * \brief Get the shared_ptr store in the UntypedSharedPtr.
         *
         * This templated function returns the std::shared_ptr hidden in the
         * UntypedSharedPtr. For the function to work, the given template
         * parameter must be the same as the one given during construction of
         * the UntypedSharedPtr. If a Derived class was given at construction, a
         * Base type can not be given to this function. The opposite (Base at
         * construction and Derived to this function) also does not work, even
         * if the actual object associated to the pointer is of type Derived.
         *
         * If the type given at construction was const, the type given to this
         * function must be const. If the type given at construction was
         * non-const, the type given to this function can either be const or
         * non- const.
         *
         * \code{.cpp}
         * class Base {};
         * class Derived : public Base {};
         *
         * // Non const example.
         * Base * ptrA = new Derived();
         * UntypedSharedPtr uspA{ptrA}; // shared pointer type is Base.
         * std::shared_ptr<Base> spA1 = uspA.getSharedPtr<Base>(); // OK
         * std::shared_ptr<const Base> spA2 = uspA.getSharedPtr<const Base>();
         * // OK
         * // std::shared_ptr<Derived> spA3 = uspA.getSharedPtr<Derived>(); //
         * KO
         *
         * // Const example
         * const Base * ptrB = new Derived();
         * UntypedSharedPtr uspB{ptrA}; // shared pointer type is const Base.
         * //std::shared_ptr<Base> spB1 = uspB.getSharedPtr<Base>(); // KO
         * std::shared_ptr<const Base> spB2 = uspB.getSharedPtr<const Base>();
         * // OK
         * // std::shared_ptr<Derived> spB3 = uspB.getSharedPtr<Derived>(); //
         * KO \endcode
         *
         * \tparam T Type of the retrieved std::shared_ptr<T>.
         * \return the std::shared_ptr<T> of the pointer stored in the
         * UntypedSharedPtr.
         * \throws std::runtime_exception if the template parameter differs
         * from the type given at construction of the UntypedSharedPtr.
         */
        template <typename T> std::shared_ptr<T> getSharedPointer() const
        {
            const auto& templateType = typeid(T);
            const auto& templateTypeNoConst = typeid(std::remove_const_t<T>);
            const auto& ownType = this->getType();
            const auto& templatePtrType = typeid(T*);
            const auto& templatePtrTypeNoConst =
                typeid(std::remove_const_t<T>*);
            const auto& ownPtrType = this->getPtrType();

            // If pointer types are identical (which includes const qualifier),
            // go for it. Unless non-pointers types are different, which may be
            // the case for arrays
            if (ownPtrType == templatePtrType && templateType == ownType) {
                std::shared_ptr<const Model<T>> typedPtr =
                    std::dynamic_pointer_cast<const Model<T>>(
                        this->sharedPtrContainer);
                if (typedPtr != NULL) {
                    return typedPtr->sharedPtr;
                }
            }

            // If pointer types are identical when loosing const qualifier of
            // function template type, go for it. Unless non-pointers types are
            // different, which may be the case for arrays
            if (ownPtrType == templatePtrTypeNoConst &&
                templateType == ownType) {
                std::shared_ptr<const Model<std::remove_const_t<T>>> typedPtr =
                    std::dynamic_pointer_cast<
                        const Model<std::remove_const_t<T>>>(
                        this->sharedPtrContainer);
                if (typedPtr != NULL) {
                    return typedPtr->sharedPtr;
                }
            }

            // If template type and owntype are the same, go for it.
            // (it is not the first tested case because constness is lost in
            // template type and own type.
            if (templateType == ownType) {
                std::shared_ptr<const Model<T>> typedPtr =
                    std::dynamic_pointer_cast<const Model<T>>(
                        this->sharedPtrContainer);
                if (typedPtr != NULL) {
                    return typedPtr->sharedPtr;
                }
            }

            // If template type and owntype are identical when loosing const
            // qualifier of function template type, go for it. (it is not the
            // first tested case because constness is lost in template type and
            // own type.
            if (templateTypeNoConst == ownType) {
                std::shared_ptr<const Model<std::remove_const_t<T>>> typedPtr =
                    std::dynamic_pointer_cast<
                        const Model<std::remove_const_t<T>>>(
                        this->sharedPtrContainer);
                if (typedPtr != NULL) {
                    return typedPtr->sharedPtr;
                }
            }

            // Type mismatch
            std::string msg("Cannot convert ");
            msg.append(ownPtrType.name());
            msg.append(" into ");
            msg.append(templatePtrType.name());
            msg.append(".");

            throw std::runtime_error(msg);
        }

        /**
         * \brief Get an empty destructor function for any type.
         *
         * When building an UntypedSharedPtr, this static method can be used to
         * provide an empty destructor for the second argument of the
         * constructor. The template param should be the same as the type of the
         * pointer given to the UntypedSharetPtr constructor.
         *
         * \tparam T the type of the pointer to deallocate.
         * \return the lambda function for deallocating.
         */
        template <typename T> static std::function<void(T*)> emptyDestructor()
        {
            return [](T* ptr) {};
        };

        /**
         * \brief Shared container containing the data structure containing the
         * actual std::shared_ptr.
         */
        std::shared_ptr<const Concept> sharedPtrContainer;
    };
} // namespace Data
#endif // !UNTYPED_SHARED_PTR_H
