#ifndef TPG_EDGE_KEYED_H
#define TPG_EDGE_KEYED_H

#include <atomic>
#include <cstddef>

#include "tpg/tpgEdge.h"

namespace TPG {

    /**
     * \brief Keyed TPGEdge to make edges traversable only by TPGTeam with a
     * specific key.
     */
    class TPGEdgeKeyed : public TPGEdge
    {
      public:
        /**
         * @brief Get lock used to control the execution of the TPGEdge.
         *
         * @return lock value used to control the execution of the TPGEdge.
         */
        uint64_t getLock() const;

        /**
         * @brief Set lock used to control the execution of the TPGEdge.
         *
         * @param newLock New lock value to set. Each lock is a product of prime
         * numbers which unlocks the execution of TPGTeamKeyed.
         */
        void setLock(uint64_t newLock);

        /**
         * @brief Check if a given key unlocks the execution of the TPGEdge.
         *
         * @param[in] key The key to check against the lock of the TPGEdge.
         * @return true if the key unlocks the execution of the TPGEdge, false
         * otherwise.
         */
        bool isUnlockedByKey(uint64_t key) const;

        /**
         * @brief Default constructor.
         *
         * Lock is initialized to 1 which make this TPGEdg traversable by
         * any TPGTeamKeyed with a key greater than 1. The lock should be
         * set to a multiple of the prime number key of the TPGTeamKeyed to
         * unlock the execution of this TPGEdgeKeyed.
         */

        TPGEdgeKeyed(const TPGVertex* src, const TPGVertex* dest,
                     const std::shared_ptr<Program::Program> prog)
            : TPGEdge(src, dest, prog), lock{1}
        {
        }

      protected:
        /// Lock used to control the execution of the TPGEdge. Each
        /// lock is a product of prime numbers which unlocks the execution of
        /// TPGEdgeKeyed
        std::uint64_t lock;
    };
} // namespace TPG

#endif // !TPG_EDGE_KEYED_H
