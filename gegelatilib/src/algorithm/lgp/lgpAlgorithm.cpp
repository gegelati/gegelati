
#include "algorithm/lgp/lgpAlgorithm.h"



std::unique_ptr<Algorithm::Algorithm> Algorithm::LGP::LGPAlgorithm::copy() const
{
    return std::make_unique<LGPAlgorithm>(this->params, this->iSet, this->algorithmName);
}

 const Environment& Algorithm::LGP::LGPAlgorithm::getEnvironment() const
{
    return *this->env;
}

void Algorithm::LGP::LGPAlgorithm::initManager()
{
    this->manager = std::make_unique<LGP::LGPManager>(*this->env, *this->outputs, this->algorithmID);
}

void Algorithm::LGP::LGPAlgorithm::initMutator()
{
    this->mutator = std::make_unique<LGP::LGPMutator>(*this->selector, this->algorithmID);
}


void Algorithm::LGP::LGPAlgorithm::initAlgorithm(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    this->env = std::make_unique<Environment>(iSet, params, dataSource);
    Algorithm::Algorithm::initAlgorithm(rng, outputs, dataSource, graph);
}



std::shared_ptr<Algorithm::PolicyStats> Algorithm::LGP::LGPAlgorithm::createPolicyStats() const
{
    std::map<std::string, std::shared_ptr<PolicyStats>> subPolicyStatsMap;
    return std::make_shared<LGPPolicyStats>(this->algorithmName, this->algorithmID, *this->env);
}

void Algorithm::LGP::LGPAlgorithm::printAgent(const Agent& agent, FILE* pFile, std::string offset, std::set<uint64_t>& printedAgentID, std::vector<std::reference_wrapper<const EvoGraph::Element>>& elementsToPrint) const
{
    if(printedAgentID.find(agent.getAgentID()) == printedAgentID.end() && this->containsAgent(agent)){
        printedAgentID.insert(agent.getAgentID());

        const LGPAgent& lgpAgent = dynamic_cast<const LGPAgent&>(agent);

        std::string constantInfo;
        std::string instructionInfo;

        // add next the content of the constant data handler in a comment (//)
        for (int i = 0; i < params.nbProgramConstant;
            i++) {
            constantInfo += std::to_string(static_cast<double>(lgpAgent.getConstantAt(i))) + "|";
        }

        // print the program instructions:
        for (int i = 0; i < lgpAgent.getNbLines(); i++) {
            const LGPLine& l = lgpAgent.getLine(i);
            // instruction index
            instructionInfo += std::to_string(l.getInstructionIndex());
            instructionInfo += "|";
            // instruction destination index
            instructionInfo += std::to_string(l.getDestinationIndex());
            instructionInfo += "&";
            // instruction operands
            for (int j = 0; j < l.getEnvironment().getMaxNbOperands(); j++) {
                std::pair<uint64_t, uint64_t> p = l.getOperand(j);
                if (j != 0)
                    instructionInfo += "#";
                instructionInfo += std::to_string(p.first);
                instructionInfo += "|";
                instructionInfo += std::to_string(p.second);
            }

            instructionInfo += "&#92;n";
        }
        fprintf(pFile,
                "%sP%" PRIu64 " [fillcolor=\"%s\" shape=diamond margin=0.03 "
                "width=0 height=0 label=\"%s.%" PRIu64 "\" constant=\"%s\" instruction=\"%s\"]\n",
                offset.c_str(), lgpAgent.getAgentID(), this->algorithmColor.c_str(), this->algorithmName.c_str(), this->algorithmID, constantInfo.c_str(), instructionInfo.c_str());
    }
}




const std::string Algorithm::LGP::LGPAlgorithm::lgpAgentRegex(
    "P([0-9]+)\\x20\\x5B.*label=\"(.*)\".*constant=\"(.*)\".*instruction=\"(.*)\"\\x5D");

const Algorithm::Agent& Algorithm::LGP::LGPAlgorithm::readAgent(std::smatch& matches)
{   
    std::regex testLgpAgentRegex(this->lgpAgentRegex);
    std::smatch newMatches;
    std::string line = matches[0];
    if(!std::regex_search(line, newMatches, testLgpAgentRegex)){
        throw std::runtime_error("LGPAlgorithm::readAgent: regex search should succeed.");
    }

    const Agent& agent = this->manager->createAgent(*graph);
    LGPManager& lgpManager = dynamic_cast<LGPManager&>(*this->manager);

    std::string constantStr = newMatches[3];
    // read constants
    std::vector<Data::Constant> v_constant;
    std::string::size_type pos = 0;
    std::string::size_type pos1 = constantStr.find("|", pos);


    for (;;) {
        if (pos1 != std::string::npos) {
            v_constant.push_back(
                {std::stod(constantStr.substr(pos, pos1 - pos))});
        }
        else {
            break;
        }
        pos = pos1 + 1;
        pos1 = constantStr.find("|", pos);

    }
    // Set the constant.
    for (int i = 0; i < v_constant.size(); i++) {
        lgpManager.setConstantAt(agent, i, v_constant[i]);
    }

    lgpManager.readLines(newMatches[4], agent);
    return agent;
}

void Algorithm::LGP::LGPAlgorithm::printCodeGenAgents(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Agent>>& agents, std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>>& subAgents) const
{
    LGPCodeGenerationEngine engine(fileMain, fileMainH, *this->env, *this->outputs, this->algorithmID, this->algorithmName);

    engine.initGlobalVar();

    for(const Agent& agent: agents) {
        engine.setExecutedAgent(agent);
        engine.generateProgram();
    }
}