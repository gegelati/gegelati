
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
            Archive* archive;


        public:

            /**
             * \brief Constructor enabling storing elements in the job so that the
             * Learning Agents will be able to use them later.
             *
             * @param[in] agent The agent that will be encapsulated into the job.
             * @param[in] manager The agent Manager of the agent.
             * @param[in] selector The selector responsible of the agent.
             * @param[in] idx The index of this job.
             * @param[in] archive The archive associated to this job.
             */
            TPGJob(std::shared_ptr<const Agent> agent, std::shared_ptr<const AgentManager> manager, std::shared_ptr<const Selector::Selector> selector,
                uint64_t idx = 0, Archive* archive = nullptr)
                : Job(agent, manager, selector, idx), archive(archive)
            {
            }

            /**
             * \brief Getter of archive.
             *
             * @return The archive of the job.
             */
            Archive& getArchive() const;

    };
};

#endif