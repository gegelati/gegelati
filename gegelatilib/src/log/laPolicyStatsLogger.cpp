#include "learn/learningAgent.h"

#include "log/laPolicyStatsLogger.h"

void Log::LAPolicyStatsLogger::logNewGeneration(uint64_t& generationNumber)
{
    this->generationNumber = generationNumber;
}

void Log::LAPolicyStatsLogger::logAfterDecimate(const TPG::TPGGraph& tpg)
{
    if (this->learningAgent.getBestRoot().first != this->lastBestRoot) {
        // Update the best root befor loggin it PolicyStats
        this->lastBestRoot = this->learningAgent.getBestRoot().first;
        *this << "Generation " << this->generationNumber << std::endl
              << std::endl;
        TPG::PolicyStats ps;
        ps.setEnvironment(this->learningAgent.getTPGGraph().getEnvironment());
        ps.analyzePolicy(this->lastBestRoot);
        *this << ps << std::endl;
        *this << std::endl
              << std::endl
              << "==========" << std::endl
              << std::endl;
    }
}
