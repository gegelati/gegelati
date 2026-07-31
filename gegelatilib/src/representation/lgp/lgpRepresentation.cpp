
#include "representation/lgp/lgpRepresentation.h"



std::unique_ptr<Representation::Representation> Representation::LGP::LGPRepresentation::copy() const
{
    return std::make_unique<LGPRepresentation>(this->iSet, 
        std::make_unique<RepresentationParameters>(*this->params), this->representationName, this->representationColor);
}

 const Representation::LGP::LGPEnvironment& Representation::LGP::LGPRepresentation::getEnvironment() const
{
    return *this->env;
}

void Representation::LGP::LGPRepresentation::initPopulation()
{
    this->population = std::make_unique<LGP::LGPPopulation>(*this->env, *this->outputs, this->representationID);
}

void Representation::LGP::LGPRepresentation::initMutator()
{
    this->mutator = std::make_unique<LGP::LGPMutator>(*this->selector, this->representationID);
}


void Representation::LGP::LGPRepresentation::initRepresentation(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    this->env = std::make_unique<LGPEnvironment>(iSet, params->lgp.nbRegisters, params->lgp.nbProgramConstant, dataSource);
    Representation::Representation::initRepresentation(rng, outputs, dataSource, graph);
}

void Representation::LGP::LGPRepresentation::clearUnusedAgentParts() 
{
    LGPPopulation& lgpPopulation = dynamic_cast<LGPPopulation&>(*this->population);
    lgpPopulation.clearAgentsIntrons();
}



std::shared_ptr<Representation::PolicyStats> Representation::LGP::LGPRepresentation::createPolicyStats() const
{
    std::map<std::string, std::shared_ptr<PolicyStats>> subPolicyStatsMap;
    return std::make_shared<LGPPolicyStats>(this->representationName, this->representationID, *this->env);
}

void Representation::LGP::LGPRepresentation::printAgent(const Individual& agent, FILE* pFile, std::string offset, std::set<uint64_t>& printedAgentID, std::vector<std::reference_wrapper<const EvoGraph::Element>>& elementsToPrint) const
{
    if(printedAgentID.find(agent.getAgentID()) == printedAgentID.end() && this->containsAgent(agent)){
        printedAgentID.insert(agent.getAgentID());

        const LgpIndividual& lgpIndividual = dynamic_cast<const LgpIndividual&>(agent);

        std::string constantInfo;
        std::string instructionInfo;

        // add next the content of the constant data handler in a comment (//)
        for (int i = 0; i < params->lgp.nbProgramConstant;
            i++) {
            constantInfo += std::to_string(static_cast<double>(lgpIndividual.getConstantAt(i))) + "|";
        }

        // print the program instructions:
        for (int i = 0; i < lgpIndividual.getNbLines(); i++) {
            const LGPLine& l = lgpIndividual.getLine(i);
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
                offset.c_str(), lgpIndividual.getAgentID(), this->representationColor.c_str(), this->representationName.c_str(), this->representationID, constantInfo.c_str(), instructionInfo.c_str());
    }
}




const std::string Representation::LGP::LGPRepresentation::lgpIndividualRegex(
    "P([0-9]+)\\x20\\x5B.*label=\"(.*)\".*constant=\"(.*)\".*instruction=\"(.*)\"\\x5D");

const Representation::Individual& Representation::LGP::LGPRepresentation::readAgent(std::smatch& matches)
{   
    std::regex testLgpIndividualRegex(this->lgpIndividualRegex);
    std::smatch newMatches;
    std::string line = matches[0];
    if(!std::regex_search(line, newMatches, testLgpIndividualRegex)){
        throw std::runtime_error("LGPRepresentation::readAgent: regex search should succeed.");
    }

    const Individual& agent = this->population->createAgent(*graph);
    LGPPopulation& lgpPopulation = dynamic_cast<LGPPopulation&>(*this->population);

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
        lgpPopulation.setConstantAt(agent, i, v_constant[i]);
    }

    lgpPopulation.readLines(newMatches[4], agent);
    return agent;
}

void Representation::LGP::LGPRepresentation::printCodeGenAgents(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Individual>>& agents, std::map<uint64_t, std::set<std::reference_wrapper<const Individual>>>& subAgents) const
{
    LGPCodeGenerationEngine engine(fileMain, fileMainH, *this->env, *this->outputs, this->representationID, this->representationName);

    engine.initGlobalVar();

    for(const Individual& agent: agents) {
        engine.setExecutedAgent(agent);
        engine.generateProgram();
    }
}