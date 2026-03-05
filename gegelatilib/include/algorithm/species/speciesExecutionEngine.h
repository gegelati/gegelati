
#ifndef SPECIES_EXECUTION_ENGINE_H
#define SPECIES_EXECUTION_ENGINE_H

#include "algorithm/executionEngine.h"
#include "algorithm/species/speciesAgent.h"
#include "algorithm/species/speciesJob.h"
#include "evoGraph/action.h"
#include "evoGraph/vertex.h"
#include "algorithm/tpg/archive.h"

#include "util/activationFunctions.h"

namespace Algorithm::Species {
    /**
     * \brief class used to execute the agent of an algorithm
     */
    class SpeciesExecutionEngine : public ExecutionEngine {

    protected:

        /// Execution engine used to execute the program of the Species agents
        ExecutionEngine* programExecutionEngine = nullptr;

        /// Archive used by the program agents.
        Archive* archive = nullptr;

        /// Action values selected
        std::vector<double> actionValues;

        /// Last values outputted by the program
        std::vector<double> lastValues;

        /// Root vertex of the species
        const EvoGraph::Vertex& rootVertex;

    public:

        /**
         * \brief SpeciesExecutionEngine constructor.
         * 
         * \param[in] rootVertex Root vertex of the species
         * \param[in] outputs outputs that will be usable for
         * interacting with this LearningEnviromnent.
         * \param[in] algorithmID id of the algorithm used.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        SpeciesExecutionEngine(const EvoGraph::Vertex& rootVertex, const Output::OutputHandler& outputs, uint64_t algorithmID, bool isTraining = false): ExecutionEngine(outputs, algorithmID, isTraining), rootVertex{rootVertex} {}

        /**
         * \brief SpeciesExecutionEngine constructor. 
         * 
         * \param[in] rootVertex Root vertex of the species
         * \param[in] executedAgent the agent to execute.
         * \param[in] outputs outputs that will be usable for
         * interacting with this LearningEnviromnent.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        SpeciesExecutionEngine(const EvoGraph::Vertex& rootVertex, const Agent& executedAgent, const Output::OutputHandler& outputs, bool isTraining = false): ExecutionEngine(executedAgent, outputs, isTraining), rootVertex{rootVertex} {}

        /**
         * Setter for the archive
         * 
         * \param[in] archive Archive used by the program agents.
         */
        void setArchive(Archive& archive);

        /**
         * \brief getter for the archive.
         */
        Archive& getArchive();

        /**
         * \brief Setup the execution engine with the given job.
         * 
         * \param[in] job the job to setup the execution engine with.
         */
        void setupJob(const Job& job) override;
        
        /**
         * \brief Set the program execution engine associated with the Species agents.
         * 
         * \param[in] programExecutionEngine the program execution engine.
         */
        void setProgramExecutionEngine(std::unique_ptr<ExecutionEngine> programExecutionEngine);

        /**
         * \brief Get the program execution engine associated with the Species agents.
         * 
         * \return the program execution engine.
         */
        ExecutionEngine& getProgramExecutionEngine() const { return *this->programExecutionEngine; }

        /**
         * \brief set the continuous action values based on last values outputted.
         */
        virtual void setContinuousActionValues();

        /**
         * \brief Execute the Program and returns the obtained double.
         *
         * If an Archive is associated to the ExecutionEngine, the Program
         * result is recorded in it.
         *
         * If the value returned by the Program is NaN, then it is replaced with
         * a -inf value.
         *
         * \param[in] program Program evaluated.
         * \return the double value returned by the Program of the Edge.
         */
        virtual double evaluateProgram(const Agent& program);

        /**
         * \brief Evaluate recursively all the vertexs of the species
         *
         * \param[in] vertex the vertex whose outgoing Edge are evaluated, should be castable to vertex.
         * \param[in] depth the actual depth in the graph
         * \param[in] mapActionEdgeProgram the map linking the edges to the programs
         * \param[in] mapContextEdgeProgram the map linking the edges to the programs
         * \param[in] actionLinks the map linking the actionId to the action values
         */
        virtual void evaluateTeam(
            const EvoGraph::Vertex& vertex, size_t depth, 
            const std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Agent>>> & mapActionEdgeProgram, 
            const std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Agent>>> & mapContextEdgeProgram, 
            const std::map<size_t, size_t>& actionLinks);

        /**
         * \brief Execute the Graph starting from the vertex pointed by the species, with the program of the agent.
         */
        virtual std::vector<double> execute() override; 


        /**
         * \brief Inherrited from ExecutionEngine
         */
        virtual const std::vector<std::reference_wrapper<const Data::DataHandler>>&
        getDataSources() const override;
        
        
    }; 
}; // namespace Algorithm::Species

#endif // SPECIES_EXECUTION_ENGINE_H