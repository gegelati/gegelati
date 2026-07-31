
#ifndef TPG_JOB_H
#define TPG_JOB_H

#include "representation/job.h"
#include "representation/tpg/tpgIndividual.h"
#include "representation/tpg/tpgPopulation.h"


namespace Representation::TPG {

    /**
     * Class for creating a job for the TPG representation
     */
    class TPGJob : public Job {

        protected:
            /**
             * Seed that will be used to randomize archive.
             */
            std::unique_ptr<TPGArchive> archive;


        public:

            /**
             * \brief Constructor enabling storing elements in the job so that the
             * Learning Agents will be able to use them later.
             *
             * @param[in] agent The agent that will be encapsulated into the job.
             * @param[in] idx The index of this job.
             * @param[in] archive The archive associated to this job.
             */
            TPGJob(const Individual& agent,
                uint64_t idx = 0, std::unique_ptr<TPGArchive> archive = nullptr)
                : Job(agent, idx), archive(std::move(archive))
            {
            }

            /**
             * \brief Getter of archive.
             *
             * @return The archive of the job.
             */
            TPGArchive& getArchive() const;

    };
};

#endif