#ifndef DICE_PROJECT_IMAGE_RESCALER_H
#define DICE_PROJECT_IMAGE_RESCALER_H

#include <vector>
#include <cstdio>
#include "constants.h"

class ImageRescaler
{
private:
    int _inputWidth, _inputHeight, _outputWidth, _outputHeight;
    std::vector< std::vector<double> > * _input, * _output;

public:
    ImageRescaler(std::vector< std::vector<double> > * input, int output_w, int output_h);
    ImageRescaler(std::vector< std::vector<double> > * input, int output_size);

    std::vector< std::vector<double> > * rescale();

    /// Getters and Setters
    void setInput(std::vector< std::vector<double> > * new_input);
    void setOutputSize(int new_width, int new_height);
    void setOutputSize(int new_size);
    std::vector< std::vector<double> > * getOutput();
};


#endif //DICE_PROJECT_IMAGE_RESCALER_H
