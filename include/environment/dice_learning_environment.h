#ifndef DICE_PROJECT_LEARNING_ENVNMT_H
#define DICE_PROJECT_LEARNING_ENVNMT_H

#include <gegelati.h>

#include "png_reader.h"
//#include "data_exporter.h"
#include "constants.h"

#include "improvedClassificationLearningEnvironment.h"

class DiceLearningEnvironment : public Learn::ImprovedClassificationLearningEnvironment
{
protected:
    static Learn::DS * dataset_training;
    static Learn::DS * dataset_testing;
    Learn::DS * current_dataset;
    Learn::LearningMode currentMode;
//    DataExporter * _csv;

    void changeCurrentImage();

public:
    DiceLearningEnvironment();

    void doAction(uint64_t actionID) override;
    void reset(size_t seed = 0, Learn::LearningMode mode = Learn::LearningMode::TRAINING) override;
    std::vector<std::reference_wrapper<const Data::DataHandler>> getDataSources() override;
    bool isCopyable() const override;
    Learn::ImprovedClassificationLearningEnvironment* clone() const override;
    double getScore() const override;
    double defaultScore() const;
    bool isTerminal() const override;
    uint8_t getCurrentImageLabel();
    void printClassifStatsTable(const Environment& env, const TPG::TPGVertex* bestRoot);

    void printTable() const;
    std::vector< std::vector<double> > getDataset();
};


#endif //DICE_PROJECT_LEARNING_ENVNMT_H
