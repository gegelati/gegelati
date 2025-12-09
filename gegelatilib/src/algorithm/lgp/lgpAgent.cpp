
#include "algorithm/lgp/lgpAgent.h"


Algorithm::LGP::LGPAgent::~LGPAgent()
{
    while (!lines.empty()) {
        lines.pop_back();
    }
}

size_t Algorithm::LGP::LGPAgent::getNbOutputs() const
{
    return this->nbOutputs;
}

Algorithm::LGP::LGPLine& Algorithm::LGP::LGPAgent::addNewLine()
{
    return this->addNewLine(this->getNbLines());
}

Algorithm::LGP::LGPLine& Algorithm::LGP::LGPAgent::addNewLine(uint64_t idx)
{
    if (idx > this->getNbLines()) {
        throw std::out_of_range(
            "Attempting to insert a line beyond the program end.");
    }
    // Allocate the zero-filled memory
    LGPLine* newLine = new LGPLine(this->environment);
    // new line is not marked as an intron by default
    this->lines.insert(this->lines.begin() + idx, {std::shared_ptr<LGPLine>(newLine), false});

    return *newLine;
}

void Algorithm::LGP::LGPAgent::addNewLine(LGPLine newLine)
{
    LGPLine* newLinePtr = new LGPLine(newLine);
    // new line is not marked as an intron by default
    this->lines.push_back({std::shared_ptr<LGPLine>(newLinePtr), false});
}

void Algorithm::LGP::LGPAgent::clearIntrons()
{
    size_t index = 0;

    // Scan the lines of the Program.
    while (index < lines.size()) {
        // If the LGPLine is an intron
        if (this->isIntron(index)) {
            // Remove it
            this->removeLine(index);
            // Do not increment index
        }
        else {
            // Next line
            index++;
        }
    }
}

void Algorithm::LGP::LGPAgent::removeLine(const uint64_t idx)
{
    this->lines.erase(this->lines.begin() + idx);
}

void Algorithm::LGP::LGPAgent::swapLines(const uint64_t idx0, const uint64_t idx1)
{
    if (idx0 >= this->getNbLines() || idx1 >= this->getNbLines()) {
        throw std::out_of_range(
            "Attempting to swap a line beyond the program end.");
    }

    std::iter_swap(this->lines.begin() + idx0, this->lines.begin() + idx1);
}

const std::shared_ptr<const Environment>& Algorithm::LGP::LGPAgent::getEnvironment() const
{
    return this->environment;
}

size_t Algorithm::LGP::LGPAgent::getNbLines() const
{
    return this->lines.size();
}

const Algorithm::LGP::LGPLine& Algorithm::LGP::LGPAgent::getLine(uint64_t index) const
{
    return *this->lines.at(index)
                .first; // throws std::out_of_range on bad index.
}

Algorithm::LGP::LGPLine& Algorithm::LGP::LGPAgent::getLine(uint64_t index)
{
    return *this->lines.at(index)
                .first; // throws std::out_of_range on bad index.
}

bool Algorithm::LGP::LGPAgent::isIntron(uint64_t index) const
{
    return this->lines.at(index)
        .second; // throws std::out_of_range on bad index.
}

void Algorithm::LGP::LGPAgent::setIntronValue(uint64_t index, bool isIntron)
{
    this->lines.at(index).second = isIntron; 
}

const Data::ConstantHandler& Algorithm::LGP::LGPAgent::cGetConstantHandler() const
{
    return this->constants;
}

const Data::Constant Algorithm::LGP::LGPAgent::getConstantAt(size_t index) const
{
    std::shared_ptr<const Data::Constant> value =
        this->constants.getDataAt(typeid(Data::Constant), index)
            .getSharedPointer<const Data::Constant>();
    return *value;
}