
#ifndef SPECIES_JOB_H
#define SPECIES_JOB_H

#include "algorithm/job.h"
#include "algorithm/species/speciesAgent.h"
#include "algorithm/species/speciesManager.h"


namespace Algorithm::Species {

    /**
     * Class for creating a job for the Species algorithm
     */
    class SpeciesJob : public Job {

        protected:
            /**
             * Seed that will be used to randomize archive.
             */
            std::unique_ptr<Archive> archive;


        public:

            /**
             * \brief Constructor enabling storing elements in the job so that the
             * Learning Agents will be able to use them later.
             *
             * @param[in] agent The agent that will be encapsulated into the job.
             * @param[in] idx The index of this job.
             * @param[in] archive The archive associated to this job.
             */
            SpeciesJob(const Agent& agent,
                uint64_t idx = 0, std::unique_ptr<Archive> archive = nullptr)
                : Job(agent, idx), archive(std::move(archive))
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