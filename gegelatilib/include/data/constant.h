
#ifndef CONSTANT_H
#define CONSTANT_H

#include <cstdint>

namespace Data {
    /**
     * \brief Data type used in Program::Program to define constant values,
     * accessible to Instructions, and mutated during the training process.
     */
    struct Constant
    {
        /**
         *	\brief the value of the Constant
         */
        int32_t value;

        /**
         *	\brief const casts of a Constant to a 32 bits integer
         */
        operator int32_t() const;

        /**
         *	\brief const casts of a Constant to a double
         */
        operator double() const;

        /**
         * \brief Comparison operator for Constant.
         */
        bool operator==(const Constant& other) const;

        /**
         * \brief Comparison operator for Constant.
         */
        bool operator!=(const Constant& other) const;
    };
} // namespace Data

#endif // CONSTANT_H