#include <cstdlib>
#include <cstdio>
#include <dirent.h>
#include <cstring>

#include <gegelati.h>

//#include "../include/evaluator.h"
#include "../include/environment/improvedClassificationLearningAgent.h"
#include "../include/environment/dice_learning_environment.h"

int howManyGraphs()
{
    int graphCount = 0;
    DIR * d;
    struct dirent * entry;
    std::string extension = "dot";

    d = opendir("../../graphsToImport/");

    while((entry = readdir(d)) != nullptr)
    {
        std::string fname = entry->d_name;

        if(entry->d_type == DT_REG && fname.find(extension, (fname.length() - extension.length())) != std::string::npos)
            graphCount++;
    }

    return graphCount;
}

std::vector<std::pair<std::string, std::string>> * getFilenames()
{
    auto files = new std::vector<std::pair<std::string, std::string>>();

    std::string path = "../../graphsToImport/", extension = "dot";
    DIR * d;
    struct dirent * entry;

    d = opendir(path.c_str());

    while((entry = readdir(d)) != nullptr)
    {
        std::string fname = entry->d_name;
        if(entry->d_type == DT_REG && fname.find(extension, (fname.length() - extension.length())) != std::string::npos)
        {
            files->push_back(*new std::pair<std::string, std::string>(path + fname, fname));
        }
    }

    return files;
}

int main()
{
    int nbGraphs = howManyGraphs();
    std::string folderPath = "../graphsToImport";

    std::cout << "How many graphs to evaluate : " << nbGraphs << std::endl;

    if(nbGraphs <= 0)
    {
        std::cout << "There is no graph to evaluate." << std::endl;
        return 0;
    }

    auto files = getFilenames();

    // -----------------------------------------------------------------------------------------------------------------
    // ------------------------------------- Set up your own Gegelati environment --------------------------------------
    // -----------------------------------------------------------------------------------------------------------------



    // -------------------------------------------- This is one for example --------------------------------------------

    Instructions::Set set;

    // Make the instruction set
    {
        auto max = [](double a, double b)->double {return std::max(a, b); };
        auto minus = [](double a, double b)->double {return a - b; };
        auto add = [](double a, double b)->double {return a + b; };
        auto sobelMagn = [](const double a[3][3])->double
        {
            double gx = -a[0][0] + a[0][2] - 2.0 * a[1][0] + 2.0 * a[1][2] - a[2][0] + a[2][2];
            double gy = -a[0][0] - 2.0 * a[0][1] - a[0][2] + a[2][0] + 2.0 * a[2][1] + a[2][2];
            return sqrt(gx * gx + gy * gy);
        };
        auto sobelDir = [](const double a[3][3])->double
        {
            double gx = -a[0][0] + a[0][2] - 2.0 * a[1][0] + 2.0 * a[1][2] - a[2][0] + a[2][2];
            double gy = -a[0][0] - 2.0 * a[0][1] - a[0][2] + a[2][0] + 2.0 * a[2][1] + a[2][2];
            return std::atan(gy / gx);
        };
        auto white = [](double a)->double {return a > 238 ? 1.0 : 0.0; };
        auto black = [](double a)->double {return a < 17 ? 1.0 : 0.0; };



        set.add(*(new Instructions::LambdaInstruction<double>(white)));
        set.add(*(new Instructions::LambdaInstruction<double>(black)));
        set.add(*(new Instructions::LambdaInstruction<const double[3][3]>(sobelMagn)));
        set.add(*(new Instructions::LambdaInstruction<const double[3][3]>(sobelDir)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(add)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(max)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
    }

    /// Set the parameters for the learning process
    Learn::LearningParameters params;
    File::ParametersParser::loadParametersFromJson("../../params.json", params);

    DiceLearningEnvironment diceLE;

    Learn::ImprovedClassificationLearningAgent<Learn::ParallelLearningAgent> agent(diceLE, set, params);

//    Environment env(set, diceLE.getDataSources(), params.nbRegisters, params.nbProgramConstant);

    auto env = agent.getEnvironment();

    // -----------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------

    std::vector<TPG::TPGGraph *> graphs;
    for(int g=0 ; g<nbGraphs ; g++)
        graphs.push_back(new TPG::TPGGraph(env));

    for(int g=0 ; g<nbGraphs ; g++)
        auto dot = new File::TPGGraphDotImporter(files->at(g).first.c_str(), env, *graphs.at(g));

    std::vector<TPG::TPGVertex *> roots;
    for(auto & graph : graphs)
        roots.push_back(new TPG::TPGVertex(*graph->getRootVertices().front()));

    std::vector<double> res(nbGraphs);

    for(int g=0 ; g<nbGraphs ; g++)
    {
//        res.at(g) = agent.evaluateOneRoot(roots.at(g), Learn::LearningMode::TESTING)->getResult();
        res.at(g) = agent.evaluateOneRoot(0, Learn::LearningMode::TESTING, roots.at(g))->getResult();
    }
    for(int g=0 ; g<res.size() ; g++)
        std::cout << "SCORE DU GRAPH n°" << g+1 << " (" << files->at(g).second << ") : " << res.at(g) << std::endl;

    return 0;
}
