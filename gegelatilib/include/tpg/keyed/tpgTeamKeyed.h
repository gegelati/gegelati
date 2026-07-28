#ifndef TPG_TEAM_KEYED_H
#define TPG_TEAM_KEYED_H

#include "tpg/tpgTeam.h"

namespace TPG {

    /**
     * \brief Keyed TPGTeam.
     *
     * When a Keyed TPGTeam is executed, it will only execute the TPGEdgeKeyed
     * whose lock is a multiple of its key. The key will be kept in memory for
     * subsequent teams in the graph.
     */
    class TPGTeamKeyed : public TPG::TPGTeam
    {
      public:
        /**
         * \brief Default constructor.
         *
         * This constructor initializes the default key to 1 which won't unlock
         * any TPGEdgeKeyed. The key should be set to a prime number greater
         * than 1 to unlock the execution of TPGEdgeKeyed associated to a lock
         * that is a multiple of this prime number.
         */
        TPGTeamKeyed() : TPGTeam(), keys{{1}}
        {
        }

        /**
         * \brief Get keys used to control the execution of the TPGVertex.
         */
        const std::set<uint64_t>& getKeys() const;

        /**
         *  \brief Add a key used to control the execution of the TPGVertex.
         *
         * @param newKey New key to add. Each key is a prime number which
         * unlocks the execution of TPGEdgeKeyed associated to a lock that is a
         * multiple of this prime number.
         */
        void addKey(uint64_t newKey);

      protected:
        /// Keys used to control the execution of the TPGVertex.
        /// Each key is a prime number which unlocks the execution of
        /// TPGEdgeKeyed associated to a lock that is a multiple of this prime
        /// number.
        std::set<uint64_t> keys;
    };
} // namespace TPG

#endif
