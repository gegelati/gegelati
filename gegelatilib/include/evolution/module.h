#ifndef EVOLUTION_MODULE_H
#define EVOLUTION_MODULE_H

#include <cstdint>
#include <memory>
#include <iostream>
#include <string>

#include "individual.h"
#include "evaluation/evaluator.h"
#include "representations/representation.h"
#include "evaluation/problem.h"
#include "mutation/mutator.h"
#include "fitnessAssignment/assigner.h"
#include "selection/selector.h"
#include "reproduction/replicator.h"

#include "log/logger.h"
#include "util/stopwatch.h"

/**
 * \brief namespace for object responsible of the main evolution
 */
namespace Evolution {

    /**
     * \brief abstract class for creating evolution module handling the initialization and generation process of evolutionnary algorithms.
     */
    class Module {

        protected:
            /// @brief Loggers of the module
            std::vector<std::shared_ptr<Log::Logger>> loggers;

            /// \brief the stopwatch to measure evolution time.
            Stopwatch stopwatch;

        public:
            // Default destructor
            virtual ~Module() = default;
            Module() = default;

            // Disable copying to avoid accidental copies (use references or pointers instead).
            Module(const Module&) = delete;
            Module& operator=(const Module&) = delete;

            /// Add a logger used by this module.
            void addLogger(std::shared_ptr<Log::Logger> logger);

            /// Get the vectior of attached logger.
            const std::vector<std::shared_ptr<Log::Logger>>& getLoggers() const;

            /// @brief print the summary of the current evolution 
            virtual std::string summary() const;

            /**
             * \brief Initialize the evolution module perform generations.
             * 
             * \param[in] nbGenerations the number of generations to perform
             */
            virtual void evolve(size_t nbGenerations);

            /**
             * \brief Initialize the evolution
             */
            virtual void initialize() = 0;

            /**
             * \brief Perform the required number of generations
             * 
             * \param[in] nbGenerations the number of generations to perform
             */
            virtual void doGenerations(size_t nbGenerations);

            /**
             * \brief Perform one generation
             * 
             * \param[in] generationNumber the index of the generation
             */
            virtual void doGeneration(size_t generationNumber) = 0;
    };
};

#endif // EVOLUTION_MODULE_H