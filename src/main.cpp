#include <cstdlib>
#include <cstdio>
#include <dirent.h>
#include <cstring>

#include "../include/evaluator.h"
#include "../include/environment/improvedClassificationLearningAgent.h"

bool isEnoughGraphs(int nbGrahs)
{
    /// @todo Use dirent to implement this verification
    return true;
}

void getFilenames()
{

}

int main()
{
    int nbGraphs = 0;
    std::string folderPath = "../graphsToImport";

    std::cout << "How many graphs to evaluate : ";
    std::cin >> nbGraphs;
    std::cout << std::endl;

    if(!isEnoughGraphs(nbGraphs))
    {
        std::cout << "You didn't provide the good amount of graphs." << std::endl;
        return 0;
    }

    auto files = new std::vector<std::string>(nbGraphs);

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
    File::ParametersParser::loadParametersFromJson("/params.json", params);

    Learn::ImprovedClassificationLearningEnvironment icle;

    // -----------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------

    Evaluator e(diceLE, nbGraphs);

    for(int g=0 ; g<nbGraphs ; g++)
    {
//        e.addGraph("", env);
    }

    return 0;
}
