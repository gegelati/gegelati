
#include "evaluation/problem.h"


std::unique_ptr<Evaluation::Problem> Evaluation::Problem::cloneUniquePtr() const
{
    return NULL;
}

bool Evaluation::Problem::isCopyable() const
{
    return false;
}

const std::vector<Dimensions::Requirement>& Evaluation::Problem::getInputDimensions() const
{
    return this->inputDimensions;
}

const Dimensions::Requirement& Evaluation::Problem::getOutputDimension() const
{
    return this->outputDimension;
}

std::set<uint64_t> Evaluation::Problem::computeEvaluationHashes(
  uint64_t nbIterations, uint64_t generationNumber, Mode mode,
  uint64_t problemSeed) const
{
  std::set<uint64_t> hashes;

  // Compute baseHash
  Data::Hash<uint64_t> hasher;
  uint64_t baseHash = hasher(static_cast<int>(mode)) ^ hasher(problemSeed);
  if(mode == Mode::TRAINING) { 
      // In training, hash should take into consideration the generation number, else not (we don't want validation to change between generations).
      baseHash = hasher(generationNumber) ^ baseHash;
  }

  // Compute hash of each iteration.
  for(size_t idx = 0; idx < nbIterations; idx++) {
    uint64_t hash = hasher(idx) ^ baseHash;
    hashes.insert(hash % UINT64_MAX);
  }

  return hashes;
}


std::string Evaluation::Problem::summary() const
{
    std::ostringstream result;            
    result << "Inputs (" << this->inputDimensions.size() << "):\n";
    for (const auto& input : this->inputDimensions) {
        result << "  * " << input.summary() << "\n";
    }
    result << "\nOutput: " << this->outputDimension << "\n";
    return result.str();
}

uint64_t Evaluation::Problem::getProblemSeed() const 
{
    return this->problemSeed;
}

std::string Evaluation::modeName(Mode mode)
{
    switch(mode) {
        case Mode::TRAINING: return "training";
        case Mode::VALIDATION: return "validation";
        default: return "testing";
    }
}
