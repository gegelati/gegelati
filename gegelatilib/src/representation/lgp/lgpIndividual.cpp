
#include "representation/lgp/lgpIndividual.h"


Representation::LGP::LgpIndividual::~LgpIndividual()
{
    while (!lines.empty()) {
        lines.pop_back();
    }
}

size_t Representation::LGP::LgpIndividual::getUsedNbOutputs(const Output::OutputHandler& outputs) const
{
    size_t nbOutputs = outputs.size();
    if(outputs.size() == 1 && outputs.sizeDiscrete() == 1) {
        nbOutputs = outputs.front().getNbValues();
    }
    return nbOutputs;
}

const Output::OutputHandler& Representation::LGP::LgpIndividual::getOutputs() const
{
    return this->outputs;
}

const Representation::LGP::LGPLine& Representation::LGP::LgpIndividual::addNewLine()
{
    return this->addNewLine(this->getNbLines());
}

const Representation::LGP::LGPLine& Representation::LGP::LgpIndividual::addNewLine(uint64_t idx)
{
    if (idx > this->getNbLines()) {
        throw std::out_of_range(
            "Attempting to insert a line beyond the program end.");
    }
    // new line is not marked as an intron by default
    
    this->lines.insert(this->lines.begin() + idx, {std::make_unique<LGPLine>(this->environment), false});

    auto itLines = this->lines.begin();
    std::advance(itLines, idx);
    return *itLines->first;
}

void Representation::LGP::LgpIndividual::addNewLine(const LGPLine& newLine, uint64_t idx)
{
    // new line is not marked as an intron by default
    if (idx > this->getNbLines()) {
        throw std::out_of_range(
            "Attempting to insert a line beyond the program end.");
    }
    // new line is not marked as an intron by default
    
    this->lines.insert(this->lines.begin() + idx, {std::make_unique<LGPLine>(newLine), false});
}

void Representation::LGP::LgpIndividual::addNewLine(const LGPLine& newLine)
{
    // new line is not marked as an intron by default
    this->addNewLine(newLine, this->getNbLines());
}

void Representation::LGP::LgpIndividual::clearIntrons()
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

void Representation::LGP::LgpIndividual::removeLine(const uint64_t idx)
{
    this->lines.erase(this->lines.begin() + idx);
}

void Representation::LGP::LgpIndividual::swapLines(const uint64_t idx0, const uint64_t idx1)
{
    if (idx0 >= this->getNbLines() || idx1 >= this->getNbLines()) {
        throw std::out_of_range(
            "Attempting to swap a line beyond the program end.");
    }

    std::iter_swap(this->lines.begin() + idx0, this->lines.begin() + idx1);
}

const Representation::LGP::LGPEnvironment& Representation::LGP::LgpIndividual::getEnvironment() const
{
    return this->environment;
}

size_t Representation::LGP::LgpIndividual::getNbLines() const
{
    return this->lines.size();
}

const Representation::LGP::LGPLine& Representation::LGP::LgpIndividual::getLine(uint64_t index) const
{
    return *this->lines.at(index)
                .first; // throws std::out_of_range on bad index.
}

Representation::LGP::LGPLine& Representation::LGP::LgpIndividual::getLineForMutation(uint64_t index)
{
    return *this->lines.at(index)
                .first; // throws std::out_of_range on bad index.
}

bool Representation::LGP::LgpIndividual::isIntron(uint64_t index) const
{
    return this->lines.at(index)
        .second; // throws std::out_of_range on bad index.
}

void Representation::LGP::LgpIndividual::setIntronValue(uint64_t index, bool isIntron)
{
    this->lines.at(index).second = isIntron; 
}


Data::ConstantHandler& Representation::LGP::LgpIndividual::getConstantHandler()
{
    return this->constants;
}

const Data::ConstantHandler& Representation::LGP::LgpIndividual::cGetConstantHandler() const
{
    return this->constants;
}

const Data::Constant Representation::LGP::LgpIndividual::getConstantAt(size_t index) const
{
    std::shared_ptr<const Data::Constant> value =
        this->constants.getDataAt(typeid(Data::Constant), index)
            .getSharedPointer<const Data::Constant>();
    return *value;
}

const std::vector<size_t>& Representation::LGP::LgpIndividual::getOutputIndices() const
{
    return this->outputIndices;
}
void Representation::LGP::LgpIndividual::setOutputIndex(size_t newOutputIndex, size_t location)
{
    if(newOutputIndex >= this->environment.getNbRegisters() || location > this->outputIndices.size()) {
        throw std::runtime_error("LgpIndividual::setOutputIndex: invalid index range or location range");
    }
    this->outputIndices[location] = newOutputIndex;
}