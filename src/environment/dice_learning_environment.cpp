#include "../../include/environment/dice_learning_environment.h"


dataset * DiceLearningEnvironment::dataset_training;
dataset * DiceLearningEnvironment::dataset_testing;

void DiceLearningEnvironment::changeCurrentImage()
{
    /// Use the right dataset
    this->current_dataset = (this->currentMode == Learn::LearningMode::TRAINING) ? this->dataset_training : this->dataset_testing;

    this->changeCurrentSample(this->currentMode);
}

DiceLearningEnvironment::DiceLearningEnvironment() : Learn::ImprovedClassificationLearningEnvironment(6, Learn::LearningAlgorithm::FS, IMG_SIZE)
{
    /// Filling the datasets
    this->dataset_testing = setupImages(new std::string(TEST_DIR));
    this->dataset_training = setupImages(new std::string(TRAIN_DIR));

    this->current_dataset = this->dataset_training;

    this->setDataset(this->current_dataset);

    /// Default values of attributes
    this->currentMode = Learn::LearningMode::TRAINING;
    this->currentSampleIndex = 0;
    /*
    for(int i=0 ; i<NB_CLASS ; i++)
        this->classificationTable.push_back(*new std::vector<uint64_t>(NB_CLASS));
    */

    if(EXPORT_DATA) // NOLINT
        this->_csv = new DataExporter();

    changeCurrentImage();
}

void DiceLearningEnvironment::doAction(uint64_t actionID)
{
    /// Call the inherited function
    ImprovedClassificationLearningEnvironment::doAction(actionID);
    //LearningEnvironment::doAction(actionID);

    //printf("\nACTION ID : %ld\nCLASS : %ld\n\n", actionID, this->currentClass);

    //classificationTable[this->currentClass][actionID]++;

    /// Change the current image after the action is accomplished
    this->changeCurrentImage();
}

void DiceLearningEnvironment::reset(size_t seed, Learn::LearningMode mode)
{
    /// Call the inherited function
    //ClassificationLearningEnvironment::reset(seed);
    /*
    for (std::vector<uint64_t>& perClass : this->classificationTable)
        for (uint64_t& score : perClass)
            score = 0;
    */

    //printTable();

    for(auto & i : this->classificationTable)
        for(unsigned long & j : i)
            j = 0;

    /// Manual reset of attributes
    this->currentMode = mode;

    /// Change the current image after the reset is accomplished
    this->changeCurrentImage();

    this->rng.setSeed(seed);

    //this->currentSampleIndex = -1;
}

std::vector<std::reference_wrapper<const Data::DataHandler>> DiceLearningEnvironment::getDataSources()
{
    return { this->currentSample };
}

bool DiceLearningEnvironment::isCopyable() const
{
    return true;
}

Learn::ImprovedClassificationLearningEnvironment *DiceLearningEnvironment::clone() const
{
    return new DiceLearningEnvironment(*this);
}

void DiceLearningEnvironment::printTable() const
{
    printf("\n");
    for(const auto & i : this->classificationTable)
    {
        for(unsigned long j : i)
            printf("%ld | ", j);
        printf("\n");
    }
}

double DiceLearningEnvironment::getScore() const
{
    return ImprovedClassificationLearningEnvironment::getScore();
}

bool DiceLearningEnvironment::isTerminal() const
{
    return false;
}

uint8_t DiceLearningEnvironment::getCurrentImageLabel()
{
    return (uint8_t)this->currentClass;
}

void DiceLearningEnvironment::printClassifStatsTable(const Environment &env, const TPG::TPGVertex *bestRoot)
{
    /// Print table of classif of the best
    TPG::TPGExecutionEngine tee(env, nullptr);

    /// Change the MODE of mnist
    this->reset(0, Learn::LearningMode::TESTING);

    /// Fill the table
    uint64_t classifTable[10][10] = {{0} };
    uint64_t nbPerClass[10] = { 0 };

    const int TOTAL_NB_IMAGE = 10000;
    for (int nbImage = 0; nbImage < TOTAL_NB_IMAGE; nbImage++) {
        /// Get answer
        uint8_t currentLabel = this->getCurrentImageLabel();
        nbPerClass[currentLabel]++;

        /// Execute
        auto path = tee.executeFromRoot(*bestRoot);
        auto action = (const TPG::TPGAction*)path.at(path.size() - 1);
        auto actionID = (uint8_t)action->getActionID();

        /// Increment table
        classifTable[currentLabel][actionID]++;

        /// Do action (to trigger image update)
        this->doAction(action->getActionID());
    }

    /// Print the table
    printf("\t");
    for (int i = 0; i < NB_CLASS; i++) {
        printf("%d\t   ", i);
    }
    printf("Nb\n");
    for (int i = 0; i < NB_CLASS; i++) {
        printf("%d\t", i);
        for (int j = 0; j < NB_CLASS; j++) {
            if (i == j) {
                printf("\033[0;32m");
            }
            printf("%2.1f\t |", 100.0 * (double)classifTable[i][j] / (double)nbPerClass[i]);
            if (i == j) {
                printf("\033[0m");
            }
        }
        printf("%4" PRIu64 "\n", nbPerClass[i]);
    }
    std::cout << std::endl;
}

std::vector<std::vector<double> > DiceLearningEnvironment::getDataset()
{
    return this->current_dataset->first;
}
