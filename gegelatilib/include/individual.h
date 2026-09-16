#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <cstdint>
#include <string>
#include <memory>

 
#include "graphBased/genotype.h"
#include "evaluation/evaluationResult.h"
#include "representations/representation.h"
#include "util/genericComparator.h"

struct CounterReset;
/**
 * \brief Class representing an Individual used by a population.
 * 
 * An individual is composed of GPNode, which are used to represent the genotype of the individual.
 */
class Individual
{
protected:

    /// \brief Genotype of the individual.
    std::unique_ptr<GraphBased::Genotype> genotype;

    /// @brief Representation of the individual.
    const Representations::Representation& representation;

    /// \brief Evaluation result of the individual.
    std::unique_ptr<Evaluation::EvaluationResult> result;

    /// Unique ID of the individual.
    size_t individualID;

    /// \brief Define if the 
    bool valid = false;

    /**
     * \brief Incremente the individual ID counter and return the new value.
     */
    static size_t incrementeCounter();

    /**
     * \brief Reset the individual ID counter.
     *
     * This method set the ID counter to a new value.
     * It can quickly lead to segmentation fault if not used carefully.
     */
    static void resetIndividualIDCounter();
    friend struct ::CounterReset;

public:

    /// Default polymorphic destructor
    virtual ~Individual() = default;

    /**
     * \brief Copy an Individual and return a unique_ptr.
     */
    virtual std::unique_ptr<Individual> cloneUniquePtr() const;

    /**
     * \brief Copy an Individual and return a shared_ptr.
     */
    virtual std::shared_ptr<Individual> cloneSharedPtr() const;

    /**
     * \brief Constructor for the Individual.
     * 
     * \param[in] representation characterizing the individual
     * \param[in] genotype the genotype set to the individual. Default is empty
     */
    Individual(const Representations::Representation& representation, std::unique_ptr<GraphBased::Genotype> genotype = std::make_unique<GraphBased::Genotype>()) 
        : representation{representation}, genotype(std::move(genotype)), 
            individualID(incrementeCounter()), result{std::make_unique<Evaluation::EvaluationResult>()} {
            this->updateValidity();
            };

    /**
     * \brief return the ID of the individual.
     */
    static size_t getIndividualIDCounter();

    /**
     * \brief Get the unique identifier of the Individual.
     *
     * \return the integer ID of the Individual.
     */
    virtual size_t getIndividualID() const;

    /**
     * \brief Set a new unique identifier to the Individual.
     *
     * \param[in] newID the new integer ID to set to the Individual.
     */
    virtual void setIndividualID(size_t newID);

    // Disable copying to avoid accidental copies (use references or pointers instead).
    Individual(const Individual&) = delete;
    Individual& operator=(const Individual&) = delete;

    /**
     * \brief Get the representation characterizing the individual
     */
    virtual const Representations::Representation& getRepresentation() const;

    /**
     * \brief Get the size of the Genotype.
     * 
     * \return the size of the Genotype.
     */
    virtual size_t getSize() const;

    /**
     * \brief Get the genotype of the Individual.
     */
    virtual const GraphBased::Genotype& getGenotype() const;

    /**
     * \brief Set the a new genotype for the Individual.
     * 
     * \param[in] genotype unique pointer towards the genotype set.
     */
    virtual void setGenotype(std::unique_ptr<GraphBased::Genotype> genotype);

    /**
     * \brief add an evaluationRun to the evaluationResult of the individual
     * 
     * \param[in] evaluationRun evaluation run.
     * \param[in] seed seed associated with the evaluation run.
     */
    virtual void addEvaluationRun(std::unique_ptr<Evaluation::EvaluationRun> evaluationRun, size_t seed) const;

    /**
     * \brief return the current evaluation result of the individual.
     */
    const Evaluation::EvaluationResult& getEvaluationResult() const;

    /**
     * \brief Return true if the current genotype is valid regarding the current representation
     */
    virtual bool isValid() const;

    /**
     * \brief Update the validity attribute of the individual.
     */
    virtual void updateValidity();

    /**
     * \brief execute based on the representation
     * 
     * \param[in] inputSources input sources on which the individual is executed.
     */
    virtual Data::DataValue execute(const std::vector<Data::DataView>& inputSources) const;
};

/**
 * \brief Comparison function to enable sorting of Individual with
 * STL.
 */
bool operator<(const Individual& a, const Individual& b);

/**
 * \brief Comparison function to enable sorting of Individual with
 * STL.
 */
bool operator==(const Individual& a, const Individual& b);

/**
 * \brief Comparison function to enable sorting of Individual with
 * STL.
 */
bool operator!=(const Individual& a, const Individual& b);

/**
 * \brief Comparison function to enable sorting of Individual with
 * STL.
 */
bool operator>(const Individual& a, const Individual& b);

/**
 * \brief Comparison function to enable sorting of Individual with
 * STL.
 */
bool operator<=(const Individual& a, const Individual& b);

/**
 * \brief Comparison function to enable sorting of Individual with
 * STL.
 */
bool operator>=(const Individual& a, const Individual& b);


#endif // INDIVIDUAL_H
