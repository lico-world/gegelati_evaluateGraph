#include "../../include/environment/image_rescaler.h"

ImageRescaler::ImageRescaler(std::vector<std::vector<double>> * input, int output_w, int output_h)
{
    this->setInput(input);
    this->setOutputSize(output_w, output_h);

    this->_inputWidth = 144;
    this->_inputHeight = 144;
}

ImageRescaler::ImageRescaler(std::vector<std::vector<double>> * input, int output_size)
{
    this->setInput(input);
    this->setOutputSize(output_size);

    this->_inputWidth = 144;
    this->_inputHeight = 144;
}

void ImageRescaler::setInput(std::vector<std::vector<double>> *new_input)
{
    this->_input = new_input;
}

void ImageRescaler::setOutputSize(int new_width, int new_height)
{
    this->_outputWidth = new_width;
    this->_outputHeight = new_height;
}

void ImageRescaler::setOutputSize(int new_size)
{
    this->setOutputSize(new_size, new_size);
}

std::vector<std::vector<double> > *ImageRescaler::getOutput()
{
    return this->_output;
}

std::vector<std::vector<double> > *ImageRescaler::rescale()
{
    int scaleFactorHeight = static_cast<int>(this->_inputHeight / this->_outputHeight), scaleFactorWidth = static_cast<int>(this->_inputWidth / this->_outputWidth);

    auto average = new std::vector< std::vector<double> >(this->_outputWidth);
    for(int i=0 ; i<this->_outputWidth ; i++)
        for(int j=0 ; j<this->_outputHeight ; j++)
            (*average)[i].push_back(0);

    if(scaleFactorHeight > 1 && scaleFactorWidth > 1)
    {
        for(int i=0 ; i<this->_outputWidth ; i++)
        {
            for(int j=0 ; j<this->_outputHeight ; j++)
            {
                for(int k=0 ; k<scaleFactorWidth ; k++)
                {
                    for(int l=0 ; l<scaleFactorHeight ; l++)
                    {
                        (*average)[i][j] += (*this->_input)[i*scaleFactorWidth + k][j*scaleFactorHeight + l] / (scaleFactorHeight*scaleFactorWidth);
                    }
                }
            }
        }
    }

    this->_output = average;

    return average;
}
