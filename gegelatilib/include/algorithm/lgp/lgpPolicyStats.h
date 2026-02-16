/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020 - 2021)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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

#ifndef LGP_POLICY_STATS_H
#define LGP_POLICY_STATS_H

#include "algorithm/policyStats.h"
#include "algorithm/lgp/lgpAgent.h"

namespace Algorithm::LGP {

    /**
     * Utility class for extracting statistics from a policy within a Graph.
     */
    class LGPPolicyStats : public PolicyStats
    {
      private:
        /// Environment used during analyses
        const Environment& environment;

        /// Data sources (including registers) used in the Program.
        std::vector<std::reference_wrapper<const Data::DataHandler>>
            dataSourcesAndRegisters;

      public:
        /**
         * \brief Number of time a Program was analyzed.
         *
         * When analyzing a policy, this number corresponds to
         * the number of Edge referencing a Program.
         */
        std::map<std::reference_wrapper<const LGPAgent>, size_t> nbUsePerProgram;

        /// Number of lines of analyzed Program.
        std::vector<size_t> nbLinesPerProgram;

        /// Number of intron lines of analyzed Program.
        std::vector<size_t> nbIntronPerProgram;

        /**
         * Each entry of this map associates an Instruction identifier from
         * an instruction set with the total number of times it was used in
         * analyzed Programs.
         */
        std::map<size_t, size_t> nbUsagePerInstruction;

        /**
         * Each entry of this map associates a data location with the total
         * number of times it was accessed by non-intron lines of analyzed
         * Programs.
         * Each data location is itself represented with a pair consisting of
         * the data source index, and the location within this data source.
         */
        std::map<std::pair<size_t, size_t>, size_t> nbUsagePerDataLocation;



        /// Default constructor
        LGPPolicyStats(std::string algorithmName, const Environment& env) : PolicyStats(algorithmName), environment{env} {
          this->dataSourcesAndRegisters.insert(
              dataSourcesAndRegisters.begin(),
              environment.getFakeDataSources().begin(),
              environment.getFakeDataSources().end());
        }

        /**
         * \brief Analyze the given Line.
         *
         * The method updates the following stats:
         * - Total number of usage of each Instruction.
         * - Total number of access for each location.
         *
         * \param[in] line line analized
         */
        void analyzeLine(const LGP::LGPLine& line);

        /**
         * Analyze the given LGPAgent.
         *
         * The method updates the following stats:
         * - Number of use per Program.
         * - Number of lines per Program.
         * - Number of intron lines per Program.
         *
         * For each non-intron line, the analyzeLine() method will be called.
         *
         * If a Program was already analyzed, it will not be analyzed again and
         * only the number of use per program will be updated.
         *
         * \param[in] agent the analyzed Agent.
         * \throws std::runtime_error if the given Program has incorrect lines
         * accessing for example non existing instructions.
         */
        virtual void analyzePolicy(const Agent& agent) override;

        /**
         * \brief Return the specific informations of the LGP usage.
         */
        virtual std::string specificInfos() const override;
    };

} // namespace Algorithm::TPG

#endif
