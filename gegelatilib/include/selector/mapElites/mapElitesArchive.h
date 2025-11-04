

#ifndef MAP_ELITES_ARCHIVE_H
#define MAP_ELITES_ARCHIVE_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>

#include "tpg/tpgGraph.h"
#include "learn/evaluationResult.h"

namespace Selector {
    namespace MapElites {


        class MapElitesArchive{
            protected:
                uint64_t nbBinPerDescriptor;
                uint64_t nbDescriptors;
                double minValue;
                double maxValue;
                std::vector<double> archiveLimits;
                std::vector<std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>> archive;

            public:

                MapElitesArchive(size_t nbBinPerDescriptor, size_t nbDescriptors, size_t minValue, size_t maxValue)
                    : nbBinPerDescriptor{nbBinPerDescriptor}, nbDescriptors{nbDescriptors}, minValue{minValue}, maxValue{maxValue}
                {
                    if(nbBinPerDescriptor > 0 && nbDescriptors > 0){
                        archive.resize(std::pow(nbBinPerDescriptor, nbDescriptors));
                    }

                    for (size_t idx = 1; idx <= nbBinPerDescriptor; ++idx) {
                        archiveLimits.push_back((double)idx / (double)nbBinPerDescriptor);
                    }
                }

                uint64_t size() const;

                std::pair<uint64_t, uint64_t> getDimensions() const;

                virtual const std::vector<std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>>& getAllArchive() const;

                virtual const std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>& 
                    getArchiveAt(const std::vector<uint64_t>& indices) const;



                virtual const std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>& getArchiveFromDescriptors(
                    const std::vector<double>& descriptors) const; 

                virtual void setArchiveAt(
                    const TPG::TPGVertex* vertex,
                    std::shared_ptr<Learn::EvaluationResult> eval,
                    const std::vector<uint64_t>& indices);
            
                virtual void setArchiveFromDescriptors(
                    const TPG::TPGVertex* vertex, 
                    std::shared_ptr<Learn::EvaluationResult> eval, 
                    const std::vector<double>& descriptors);  

                virtual uint64_t getIndexArchive(double value) const;
                virtual uint64_t computeLinearIndex(const std::vector<uint64_t>& indices) const;


                virtual void initCSVarchive(std::string path) const ;
                virtual void updateCSVArchive(std::string path, uint64_t generationNumber) const;

                virtual bool containsRoot(const TPG::TPGVertex* root) const;

                virtual void removeRootFromArchiveIfNotComplete(const TPG::TPGVertex* root, size_t maxNbEvaluation);
                virtual void removeRootFromArchive(const TPG::TPGVertex* root, size_t maxNbEvaluation);


    };
    

    }; // namespace MapElites
}; // namespace Selector

#endif // MAP_ELITES_SELECTOR_H