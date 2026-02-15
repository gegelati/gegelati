
#include "algorithm/lgp/lgpAlgorithm.h"



std::unique_ptr<Algorithm::Algorithm> Algorithm::LGP::LGPAlgorithm::copy() const
{
    return std::make_unique<LGPAlgorithm>(this->params, this->iSet, this->algorithmName + "_copy");
}

 std::shared_ptr<const Environment> Algorithm::LGP::LGPAlgorithm::getEnvironment() const
{
    return this->env;
}

void Algorithm::LGP::LGPAlgorithm::initManager(std::shared_ptr<const Output::OutputHandler> outputs)
{
    this->manager = std::make_shared<LGP::LGPManager>(this->env, *outputs);
    this->manager->setAlgorithmName(algorithmName);
}

void Algorithm::LGP::LGPAlgorithm::initMutator()
{
    this->mutator = std::make_shared<LGP::LGPMutator>(*this->selector);
    this->mutator->setAlgorithmName(algorithmName);
}


void Algorithm::LGP::LGPAlgorithm::initAlgorithm(RNG::RNG& rng, std::shared_ptr<const Output::OutputHandler> outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    this->env = std::make_shared<Environment>(iSet, params, dataSource);
    Algorithm::Algorithm::initAlgorithm(rng, outputs, dataSource, graph);
}

void Algorithm::LGP::LGPAlgorithm::printAgent(const Agent& agent, FILE* pFile, std::string offset, std::set<uint64_t>& printedAgentID, std::vector<std::reference_wrapper<const EvoGraph::Element>>& elementsToPrint) const
{
    if(printedAgentID.find(agent.getAgentID()) == printedAgentID.end() && this->containsAgent(agent)){
        printedAgentID.insert(agent.getAgentID());

        const LGPAgent& lgpAgent = dynamic_cast<const LGPAgent&>(agent);


        fprintf(pFile,
                "%sP%" PRIu64 " [fillcolor=\"hsl(283, 60%, 44%)\" shape=diamond margin=0.03 "
                "width=0 height=0 label=\"%s\"]\n",
                offset.c_str(), lgpAgent.getAgentID(), lgpAgent.getAlgorithmName().c_str());
        // add next the content of the constant data handler in a comment (//)
        for (int i = 0; i < params.nbProgramConstant;
            i++) {
            fprintf(pFile, "%f|", static_cast<double>(lgpAgent.getConstantAt(i)));
        }
        fprintf(pFile, "\n");

        // print the program instructions:
        std::string programContent = "";
        for (int i = 0; i < lgpAgent.getNbLines(); i++) {
            const LGPLine& l = lgpAgent.getLine(i);
            // instruction index
            programContent += std::to_string(l.getInstructionIndex());
            programContent += "|";
            // instruction destination index
            programContent += std::to_string(l.getDestinationIndex());
            programContent += "&";
            // instruction operands
            for (int j = 0; j < l.getEnvironment()->getMaxNbOperands(); j++) {
                std::pair<uint64_t, uint64_t> p = l.getOperand(j);
                if (j != 0)
                    programContent += "#";
                programContent += std::to_string(p.first);
                programContent += "|";
                programContent += std::to_string(p.second);
            }

            programContent += "&#92;n";
        }
        fprintf(pFile, "%sI%" PRIu64 " [shape=box style=invis label=\"%s\"] \n",
                offset.c_str(), lgpAgent.getAgentID(),
                programContent.c_str());

        fprintf(pFile, "%sP%" PRIu64 " -> I%" PRIu64 "[style=invis]\n",
                offset.c_str(), lgpAgent.getAgentID(), lgpAgent.getAgentID());
    }
}