
#ifndef TPG_JOB_H
#define TPG_JOB_H

#include "algorithm/job.h"
#include "algorithm/tpg/tpgAgent.h"
#include "algorithm/tpg/tpgManager.h"


namespace Algorithm::TPG {

    /**
     * Class for creating a job for the TPG algorithm
     */
    class TPGJob : public Job {

        protected:
            /**
             * Seed that will be used to randomize archive.
             */
            const uint64_t archiveSeed;

        public:

            /**
             * \brief Constructor enabling storing elements in the job so that the
             * Learning Agents will be able to use them later.
             *
             * @param[in] agent The agent that will be encapsulated into the job.
             * @param[in] manager The agent Manager of the agent.
             * @param[in] selector The selector responsible of the agent.
             * @param[in] archiveSeed The archive seed that will be used with this
             * job.
             * @param[in] idx The index of this job.
             */
            TPGJob(std::shared_ptr<const Agent> agent, std::shared_ptr<const AgentManager> manager, std::shared_ptr<const Selector::Selector> selector, uint64_t archiveSeed = 0,
                uint64_t idx = 0)
                : Job(agent, manager, selector, idx), archiveSeed(archiveSeed)
            {
            }

            /**
             * \brief Getter of archiveSeed.
             *
             * @return The archive seed of the job.
             */
            uint64_t getArchiveSeed() const;
    };
};

#endif