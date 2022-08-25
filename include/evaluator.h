#ifndef GEGELATI_EVALUATEGRAPH_EVALUATOR_H
#define GEGELATI_EVALUATEGRAPH_EVALUATOR_H

#include <vector>
#include <gegelati.h>

#include "environment/improvedClassificationLearningAgent.h"


class Evaluator
{
private:
    double evaluateOneGraph(int graphID);

protected:
    int _nbGraphs;
    std::vector< std::pair<File::TPGGraphDotImporter&, double> > * _graphs;

    Environment& _env;
    Learn::ImprovedClassificationLearningAgent<Learn::ParallelLearningAgent>& _agent;

public:
    Evaluator(Learn::ImprovedClassificationLearningAgent<Learn::ParallelLearningAgent>& agent,
              Environment& env, int nbGraphs);
    void addGraph(const char* path, Environment& env);
    std::vector<double> evaluateGraphs();
};


#endif //GEGELATI_EVALUATEGRAPH_EVALUATOR_H
