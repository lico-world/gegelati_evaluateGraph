#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#ifndef DICE_PROJECT_PNG_READER_H
#define DICE_PROJECT_PNG_READER_H

//***************************************************************************************************
//*** This code is based on the one available here : http://zarb.org/~gc/html/libpng.html ***********
//***************************************************************************************************
//*** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, ***********
//*** INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR ******
//*** PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL GUILLAUME COTTENCEAU BE LIABLE FOR ANY CLAIM, **
//*** DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING ******
//*** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. *
//***************************************************************************************************

#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <unistd.h>
#include <string>
#include <cstring>
#include <dirent.h>
#include <iostream>
#include <cstdlib>

#include <png.h>

#include "image_rescaler.h"
#include "constants.h"

#define TRAIN_DIR "../../data/train/"
#define TEST_DIR "../../data/test/"


///---------------------------------------- Static functions --------------------------------------------

using dataset = std::pair<std::vector< std::vector<double>>, std::vector<double>>;

/// Useful to abort the process if any error occurs
static void abort_(const char * s, ...);

/// Manage the reading of any PNG file
static void readPngFile(char *filename, std::vector<png_bytepp> * images);

/// Transform an array of 2D representation images in an array of 1D representation images
static dataset * linearizeArray(std::vector< std::vector< std::vector< double > > > * initialArray, std::vector<char *> *filenames);

///-------------------------------------- Non-static functions ------------------------------------------


/// Return an array of all images on the std::vector<double> format
//std::vector< std::vector<double> > * setupImages(std::vector<char *> * filenames);
dataset * setupImages(std::string * path);

#endif //DICE_PROJECT_PNG_READER_H

#pragma clang diagnostic pop