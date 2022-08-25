#include "../include/evaluator.h"

Evaluator::Evaluator(Learn::ImprovedClassificationLearningAgent<Learn::ParallelLearningAgent>& agent,
                     Environment& env, int nbGraphs) : _nbGraphs(nbGraphs), _env(env), _agent(agent)
{
    this->_graphs = new std::vector< std::pair<File::TPGGraphDotImporter&, double> >();
}

void Evaluator::addGraph(const char *path, Environment& env)
{
    auto imp = new File::TPGGraphDotImporter(path, env, *this->_agent.getTPGGraph());
    this->_graphs->push_back(std::pair<File::TPGGraphDotImporter&, double>(*imp, 0.0));
}

double Evaluator::evaluateOneGraph(int graphID)
{
    double score = 0.0;
    int good=0, total=0;

    auto results = this->_agent.evaluateAllRoots(0, Learn::LearningMode::TESTING);
    auto classif_tables = this->_agent.getClassificationTables();

    for(auto & root : classif_tables)
    {
        auto table = root.second;
        for(int i=0 ; i<table->size() ; i++)
        {
            for(int j=0 ; j<table->at(i).size() ; j++)
            {
                if(i==j)
                    good++;
                total++;
            }
        }
    }

    return (double)good / (double)total;
}

std::vector<double> Evaluator::evaluateGraphs()
{
    std::vector<double> res(this->_nbGraphs);

    for(int g=0 ; g<res.size() ; g++)
        res.at(g) = this->evaluateOneGraph(g);

    return res;
}
