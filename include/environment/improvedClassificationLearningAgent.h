#ifndef DICE_PROJECT_IMPROVEDCLASSIFICATIONLEARNINGAGENT_H
#define DICE_PROJECT_IMPROVEDCLASSIFICATIONLEARNINGAGENT_H

#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "learn/classificationEvaluationResult.h"
#include "improvedClassificationLearningEnvironment.h"
#include "learn/evaluationResult.h"
#include "learn/learningAgent.h"
#include "learn/parallelLearningAgent.h"
#include <data/hash.h>

namespace Learn {
    /**
     * \brief LearningAgent specialized for LearningEnvironments representing a
     * classification problem.
     *
     * The key difference between this ClassificationLearningAgent and the base
     * LearningAgent is the way roots are selected for decimation after each
     * generation. In this agent, the roots are decimated based on an average
     * score **per class** instead of decimating roots based on their
     * global average score (over all classes) during the last evaluation.
     * By doing so, the roots providing the best score in each class are
     * preserved which increases the chances of correct classifiers emergence
     * for all classes.
     *
     * In this context, it is assumed that each action of the
     * LearningEnvironment represents a class of the classification problem.
     *
     * The BaseLearningAgent template parameter is the LearningAgent from which
     * the ClassificationLearningAgent inherits. This template notably enable
     * selecting between the classical and the ParallelLearningAgent.
     */
    template <class BaseLearningAgent = ParallelLearningAgent>
    class ImprovedClassificationLearningAgent : public BaseLearningAgent
    {
        static_assert(
                std::is_convertible<BaseLearningAgent*, LearningAgent*>::value);

    protected:
        /**
         * \brief classificationTables saves the classification tables for each root
         *  it is mapped with the root as a key
         */
//        std::unordered_map<const TPG::TPGVertex *, std::vector<std::vector<uint64_t>>*> classificationTables;
        std::vector< std::pair< const TPG::TPGVertex *, std::vector<std::vector<uint64_t>>* > > classificationTables;

    public:
        /**
         * \brief Constructor for LearningAgent.
         *
         * \param[in] le The LearningEnvironment for the TPG.
         * \param[in] iSet Set of Instruction used to compose Programs in the
         *            learning process.
         * \param[in] p The LearningParameters for the LearningAgent.
         * \param[in] factory The TPGFactory used to create the TPGGraph. A
         * default TPGFactory is used if none is provided.
         */
        ImprovedClassificationLearningAgent(
                ImprovedClassificationLearningEnvironment& le,
                const Instructions::Set& iSet, const LearningParameters& p,
                const TPG::TPGFactory& factory = TPG::TPGFactory())
                : BaseLearningAgent(le, iSet, p, factory)
        {
//            this->classificationTables = *new std::unordered_map<const TPG::TPGVertex *, std::vector<std::vector<uint64_t>>*>();
            this->classificationTables = *new std::vector< std::pair< const TPG::TPGVertex *, std::vector<std::vector<uint64_t>>* > >();
        };

        /**
         * \brief Specialization of the evaluateJob method for classification
         * purposes.
         *
         * This method returns a ClassificationEvaluationResult for the
         * evaluated root instead of the usual EvaluationResult.
         */
        virtual std::shared_ptr<EvaluationResult> evaluateJob(
                TPG::TPGExecutionEngine& tee, const Job& root,
                uint64_t generationNumber, LearningMode mode,
                LearningEnvironment& le) const override;

        /**
         * \brief Specialization of the decimateWorstRoots method for
         * classification purposes.
         *
         * During the decimation process, roughly half of the roots are kept
         * based on their score for individual class of the
         * ClassificationLearningEnvironment. To do so, for each class of the
         * ClassificationLearningEnvironment, the roots provided the best score
         * are preserved during the decimation process even if their global
         * score over all classes is not among the best.
         *
         * The remaining half of preserved roots is selected using the general
         * score obtained over all classes.
         *
         * This per-class preservation is activated only if there is a
         * sufficient number of root vertices in the TPGGraph after decimation
         * to guarantee that all classes are preserved equally. In other word,
         * the same number of root is marked for preservation for each class,
         * which can only be achieved if the number of roots to preserve during
         * the decimation process is superior or equal to twice the number of
         * actions of the ClassificationLearningEnvironment. If an insufficient
         * number of root is preserved during the decimation process, all roots
         * are preserved based on their general score.
         *
         * The results map is updated by the method to keep only the results of
         * non-decimated roots.
         */
        void decimateWorstRoots(
                std::multimap<std::shared_ptr<EvaluationResult>,
                        const TPG::TPGVertex*>& results) override;

        /**
         * \brief Train the TPGGraph for one generation.
         *
         * Training for one generation includes:
         * - Populating the TPGGraph according to given MutationParameters.
         * - Evaluating all roots of the TPGGraph. (call to evaluateAllRoots)
         * - Removing from the TPGGraph the worst performing root TPGVertex.
         *
         * \param[in] generationNumber the integer number of the current
         * generation.
         */
        virtual void trainOneGeneration(uint64_t generationNumber) override;

        /**
         * \brief Evaluate all root TPGVertex of the TPGGraph.
         *
         * This method calls the evaluateJob method for every root TPGVertex
         * of the TPGGraph. The method returns a sorted map associating each
         * root vertex to its average score, in ascending order or score.
         *
         * \param[in] generationNumber the integer number of the current
         * generation. \param[in] mode the LearningMode to use during the policy
         * evaluation.
         */
        virtual std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*> evaluateAllRoots(uint64_t generationNumber, LearningMode mode);

        std::vector< std::pair< const TPG::TPGVertex *, std::vector<std::vector<uint64_t>>* > > getClassificationTables();
    };

    template <class BaseLearningAgent>
    inline std::shared_ptr<EvaluationResult> ImprovedClassificationLearningAgent<
            BaseLearningAgent>::evaluateJob(TPG::TPGExecutionEngine& tee,
                                            const Job& job,
                                            uint64_t generationNumber,
                                            LearningMode mode,
                                            LearningEnvironment& le) const
    {
        // Only consider the first root of jobs as we are not in adversarial
        // mode
        const TPG::TPGVertex* root = job.getRoot();

        // Skip the root evaluation process if enough evaluations were already
        // performed. In the evaluation mode only.
        std::shared_ptr<Learn::EvaluationResult> previousEval;
        if (mode == LearningMode::TRAINING && this->isRootEvalSkipped(*root, previousEval))
            return previousEval;

        // Init results
        std::vector<double> result(this->learningEnvironment.getNbActions(), 0.0);
        std::vector<size_t> nbEvalPerClass(this->learningEnvironment.getNbActions(), 0);

        // Evaluate nbIteration times
        for (auto i = 0; i < this->params.nbIterationsPerPolicyEvaluation; i++)
        {
            // Compute a Hash
            Data::Hash<uint64_t> hasher;
            uint64_t hash = hasher(generationNumber) ^ hasher(i);

            // Reset the learning Environment
            le.reset(hash, mode);

            uint64_t nbActions_onEval = 0;
            while (!le.isTerminal() && nbActions_onEval < this->params.maxNbActionsPerEval)
            {
                // Get the action
                uint64_t actionID = ((const TPG::TPGAction*)tee.executeFromRoot(*root).back())->getActionID();
                // Do it
                le.doAction(actionID);
                // Count actions
                nbActions_onEval++;
            }

            // Update results
            auto icle = dynamic_cast<Learn::ImprovedClassificationLearningEnvironment*>(&this->learningEnvironment);
            auto classificationTable = icle->getClassificationTable();

            // for each class
            for (uint64_t classIdx = 0; classIdx < classificationTable.size(); classIdx++)
            {
                uint64_t truePositive = classificationTable.at(classIdx).at(classIdx);
                uint64_t falseNegative = std::accumulate(classificationTable.at(classIdx).begin(),
                                                         classificationTable.at(classIdx).end(),
                                                         (uint64_t)0) - truePositive;

                result.at(classIdx) += le.getScore();

                nbEvalPerClass.at(classIdx) += truePositive + falseNegative;
            }
        }

        // Before returning the EvaluationResult, divide the result per class by
        // the number of iteration
        const LearningParameters& p = this->params;
        std::for_each(result.begin(), result.end(), [p](double& val) {
            val /= (double)p.nbIterationsPerPolicyEvaluation;
        });

        // Create the EvaluationResult
        auto evaluationResult = std::shared_ptr<EvaluationResult>(
                new ClassificationEvaluationResult(result, nbEvalPerClass));

        // Combine it with previous one if any
        if (previousEval != nullptr) {
            *evaluationResult += *previousEval;
        }
        return evaluationResult;
    }

    template <class BaseLearningAgent>
    void ImprovedClassificationLearningAgent<BaseLearningAgent>::decimateWorstRoots(
            std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>&
            results)
    {
        // Check that results are ClassificationEvaluationResults.
        // (also throws on empty results)
        const EvaluationResult* result = results.begin()->first.get();
        if (typeid(ClassificationEvaluationResult) != typeid(*result)) {
            throw std::runtime_error(
                    "ClassificationLearningAgent can not decimate worst roots for "
                    "results whose type is not ClassificationEvaluationResult.");
        }

        // Compute the number of root to keep/delete base on each criterion
        uint64_t totalNbRoot = this->tpg->getNbRootVertices();
        uint64_t nbRootsToDelete =
                (uint64_t)floor(this->params.ratioDeletedRoots * totalNbRoot);
        uint64_t nbRootsToKeep = (totalNbRoot - nbRootsToDelete);

        // Keep ~half+ of the roots based on their general score on
        // all class.
        // and ~half- of the roots on a per class score (none if nbRoots to keep
        // < 2*nb class)
        uint64_t nbRootsKeptPerClass =
                (nbRootsToKeep / this->learningEnvironment.getNbActions()) / 2;
        uint64_t nbRootsKeptGeneralScore =
                nbRootsToKeep -
                this->learningEnvironment.getNbActions() * nbRootsKeptPerClass;

        // Build a list of roots to keep
        std::vector<const TPG::TPGVertex*> rootsToKeep;

        // Insert roots to keep per class
        for (uint64_t classIdx = 0;
             classIdx < this->learningEnvironment.getNbActions(); classIdx++) {
            // Fill a map with the roots and the score of the specific class as
            // ID.
            std::multimap<double, const TPG::TPGVertex*> sortedRoot;
            std::for_each(
                    results.begin(), results.end(),
                    [&sortedRoot,
                            &classIdx](const std::pair<std::shared_ptr<EvaluationResult>,
                            const TPG::TPGVertex*>& res) {
                        sortedRoot.emplace(
                                ((ClassificationEvaluationResult*)res.first.get())
                                        ->getScorePerClass()
                                        .at(classIdx),
                                res.second);
                    });

            // Keep the best nbRootsKeptPerClass (or less for reasons explained
            // in the loop)
            auto iterator = sortedRoot.rbegin();
            for (auto i = 0; i < nbRootsKeptPerClass; i++) {
                // If the root is not already marked to be kept
                if (std::find(rootsToKeep.begin(), rootsToKeep.end(),
                              iterator->second) == rootsToKeep.end()) {
                    rootsToKeep.push_back(iterator->second);
                }
                // Advance the iterator no matter what.
                // This means that if a root scores well for several classes
                // it is kept only once anyway, but additional roots will not
                // be kept for any of the concerned class.
                iterator++;
            }
        }

        // Insert remaining roots to keep
        auto iterator = results.rbegin();
        while (rootsToKeep.size() < nbRootsToKeep &&
               iterator != results.rend()) {
            // If the root is not already marked to be kept
            if (std::find(rootsToKeep.begin(), rootsToKeep.end(),
                          iterator->second) == rootsToKeep.end()) {
                rootsToKeep.push_back(iterator->second);
            }
            // Advance the iterator no matter what.
            iterator++;
        }

        // Do the removal.
        // Because of potential root actions, the preserved number of roots
        // may be higher than the given ratio.
        auto allRoots = this->tpg->getRootVertices();
        auto& tpgRef = this->tpg;
        auto& resultsPerRootRef = this->resultsPerRoot;
        std::for_each(
                allRoots.begin(), allRoots.end(),
                [&rootsToKeep, &tpgRef, &resultsPerRootRef,
                        &results](const TPG::TPGVertex* vert) {
                    // Do not remove actions
                    if (dynamic_cast<const TPG::TPGAction*>(vert) == nullptr &&
                        std::find(rootsToKeep.begin(), rootsToKeep.end(), vert) ==
                        rootsToKeep.end()) {
                        tpgRef->removeVertex(*vert);

                        // Keep only results of non-decimated roots.
                        resultsPerRootRef.erase(vert);

                        // Update results also
                        std::multimap<std::shared_ptr<EvaluationResult>,
                                const TPG::TPGVertex*>::iterator iter =
                                results.begin();
                        while (iter != results.end()) {
                            if (iter->second == vert) {
                                results.erase(iter);
                                break;
                            }
                            iter++;
                        }
                    }
                });
    }


    template<class BaseLearningAgent>
    void ImprovedClassificationLearningAgent<BaseLearningAgent>::trainOneGeneration(uint64_t generationNumber)
    {
        auto icle = dynamic_cast<Learn::ImprovedClassificationLearningEnvironment*>(&this->learningEnvironment);

        for (auto logger : this->loggers) {
            logger.get().logNewGeneration(generationNumber);
        }

        // Populate Sequentially
        Mutator::TPGMutator::populateTPG(*this->tpg, this->archive,
                                         this->params.mutation, this->rng,
                                         this->maxNbThreads);
        for (auto logger : this->loggers) {
            logger.get().logAfterPopulateTPG();
        }

        // Evaluate
        auto results = this->evaluateAllRoots(generationNumber, LearningMode::TRAINING);

        // ----------------------------------------------- FS SCORE ----------------------------------------------------

        auto newResults = std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex *>();

        if(icle->getAlgo() == Learn::LearningAlgorithm::FS || icle->getAlgo() == Learn::LearningAlgorithm::BANDIT)
        {
            for(auto &root : results)
            {
                // Compute the correct score
                auto score = icle->getScore_FS(root.second, &this->classificationTables);
//                auto score = std::vector<double>(icle->getNbActions(), 1);

                auto nbEval = std::vector<size_t>(icle->getNbActions(), 0);

                for(auto & table : this->classificationTables)
                {
                    if(table.first == root.second)
                    {
                        for(int c=0 ; c<icle->getNbActions() ; c++)
                            for(int c_i=0 ; c_i<icle->getNbActions() ; c_i++)
                                nbEval.at(c) += table.second->at(c).at(c_i);
                    }
                }

                // Build a new EvaluationResult object
                auto evaluationRes = *new ClassificationEvaluationResult(score, nbEval);

                auto r = new std::pair<std::shared_ptr<ClassificationEvaluationResult>,
                                    const TPG::TPGVertex *>(std::make_shared<ClassificationEvaluationResult>(evaluationRes), root.second);
                newResults.insert(*r);
            }

            // Emplace the fake scores with the real ones
            results = newResults;
        }

        // -------------------------------------------------------------------------------------------------------------

        for (auto logger : this->loggers) {
            logger.get().logAfterEvaluate(results);
        }

        // Remove worst performing roots
        decimateWorstRoots(results);

        // Update the best
        this->updateEvaluationRecords(results);

        for (auto logger : this->loggers) {
            logger.get().logAfterDecimate();
        }

        // Does a validation or not according to the parameter doValidation
        if (this->params.doValidation) {
            auto validationResults =
                    this->evaluateAllRoots(generationNumber, Learn::LearningMode::VALIDATION);
            for (auto logger : this->loggers) {
                logger.get().logAfterValidate(validationResults);
            }
        }

        for (auto logger : this->loggers) {
            logger.get().logEndOfTraining();
        }

        // -------------------------------- Refresh the datasubset -------------------------------------------

        icle->refreshDatasubset();

        // Clear the classification tables
        this->classificationTables.clear();
    }



    template<class BaseLearningAgent>
    std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex *> ImprovedClassificationLearningAgent<BaseLearningAgent>::
            evaluateAllRoots(uint64_t generationNumber, LearningMode mode)
    {
        std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*> result;

        // Create the TPGExecutionEngine for this evaluation.
        // The engine uses the Archive only in training mode.
        std::unique_ptr<TPG::TPGExecutionEngine> tee =
                this->tpg->getFactory().createTPGExecutionEngine(this->env,  (mode == LearningMode::TRAINING) ? &this->archive : NULL);

        for (int i = 0; i < this->tpg->getNbRootVertices(); i++)
        {
            auto job = this->makeJob(i, mode);
            auto root = (*job).getRoot();
            this->archive.setRandomSeed(job->getArchiveSeed());
            std::shared_ptr<EvaluationResult> avgScore = this->evaluateJob(*tee, *job, generationNumber, mode, this->learningEnvironment);
            result.emplace(avgScore, root);

            // Save the classification table
            auto icle = dynamic_cast<Learn::ImprovedClassificationLearningEnvironment*>(&this->learningEnvironment);
            auto classificationTable = icle->getClassificationTable();

            if(icle->getAlgo() == Learn::LearningAlgorithm::FS || icle->getAlgo() == Learn::LearningAlgorithm::BANDIT)
            {
                auto tab = new std::vector< std::vector<uint64_t> >(classificationTable);
                auto p = new std::pair<const TPG::TPGVertex *, std::vector<std::vector<uint64_t>>*>(root, tab);
//                this->classificationTables.insert(*p);
                this->classificationTables.push_back(*p);
            }
        }

        return result;
    }

    template<class BaseLearningAgent>
    std::vector< std::pair< const TPG::TPGVertex *, std::vector<std::vector<uint64_t>>* > > ImprovedClassificationLearningAgent<BaseLearningAgent>::
    getClassificationTables()
    {
        return this->classificationTables;
    }
}; // namespace Learn

#endif //DICE_PROJECT_IMPROVEDCLASSIFICATIONLEARNINGAGENT_H
