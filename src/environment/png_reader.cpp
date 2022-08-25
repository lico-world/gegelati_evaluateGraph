#include "../../include/environment/png_reader.h"

static void abort_(const char * s, ...)
{
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}

static void printDebug(std::vector<char *> * debug)
{
    printf("[------------------------------------------------------------]\n");
    for(int i=0 ; i<debug->size() ; i++)
        printf("[%d] : %s\n", i, (*debug)[i]);
}

std::vector<char *> *setDataSets(const std::string *path)
{
    if(*path == "../../data/train/")
    {
        int a=5;
        a*=6;
    }

    /// Initialisation of the structure that will contain all files names
    auto names = new std::vector<char *>();

    /// Usage of the dirent library to access to whole directories
    struct dirent *dir;
    DIR *d = opendir(path->c_str());

    if(d)
    {
        /// Manage all files in the directory
        while((dir = readdir(d)) != nullptr)
        {
            /// Ignore the '.' and the '..' files because they are not meaningful here
            if(strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
            {
                /// Prefix contains the relative path '../../'
                auto prefix = new char[256];

                /// Adding the prefix then push-back the path to the path vector 'names'
                strcpy(prefix, path->c_str());
                strcat(prefix, dir->d_name);
                names->push_back(prefix);
            }
        }
        /// Close the directory after usage
        closedir(d);
    }

    return names;
}

static void readPngFile(char *filename, std::vector<png_bytepp> * images)
{
    /// The header is useful to determinate if the file is a PNG, it's the first 8 bytes of the file
    unsigned char header[8];

    /// This ptr leads to the PNG structure
    png_structp png_ptr;

    /// This ptr leads to the PNG information
    png_infop info_ptr;

    /// Open file and test for it being a png
    FILE *fp = fopen(filename, "rb");
    if(!fp)
        abort_("[read_png_file] File %s could not be opened for reading", filename);

    /// Read the header to determinate if it is a PNG file
    fread(header, 1, 8, fp);
    if (png_sig_cmp( reinterpret_cast<png_const_bytep>(header), 0, 8))
        abort_("[read_png_file] File %s is not recognized as a PNG file", filename);

    /// Make the link with the PNG structure
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
        abort_("[read_png_file] png_create_read_struct failed");

    /// Make the link with the PNG information
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        abort_("[read_png_file] png_create_info_struct failed");

    /// Initialisation of the Input-Output system
    png_init_io(png_ptr, fp);

    /// Indicate that the first 8 bytes were already processed
    png_set_sig_bytes(png_ptr, 8);

    /// Read the PNG information first
    png_read_info(png_ptr, info_ptr);

    /// Recovery the image size
    auto height = static_cast<int>(png_get_image_height(png_ptr, info_ptr));
    auto width = static_cast<int>(png_get_image_width(png_ptr, info_ptr));

    /// Now read the whole PNG image data
    auto column = new png_bytep[height];
    images->push_back(column);

    for(int i=0 ; i<height ; i++)
        column[i] = new png_byte[width];

    png_read_image(png_ptr, (*images)[images->size()-1]); // The last one because de size grows as the loop is traversed

    /// Finalization of the image reading
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, static_cast<png_infopp>(nullptr));

    /// Close the stream after utilisation
    fclose(fp);
}

static double wantedValue(char * filenames)
{
    return static_cast<double>(atof(&filenames[strlen(filenames)-11])) -1;
}

static dataset * linearizeArray(std::vector< std::vector< std::vector< double > > > * initialArray, std::vector<char *> *filenames)
{
    /// Initialisation of the new well-dimensioned array
    auto newArray = new dataset();

    /// Manage all images
    for(int img=0 ; img<static_cast<int>((*initialArray).size()) ; img++) // (*initialArray).size() is the number of images (9)
    {
        auto imgLine = new std::vector<double>();
        newArray->first.push_back(*imgLine);

        /// Manage all rows in one image
        for(int i=0 ; i<static_cast<int>((*initialArray)[img].size()) ; i++) // (*initialArray)[img].size() is the number of rows in an image (144)
        {
            /// Manage all values in one row
            for(int j=0 ; j<static_cast<int>((*initialArray)[img][i].size()) ; j++) // (*initialArray)[img][i].size() is the number of values in a row (144)
                newArray->first[img].push_back((*initialArray)[img][i][j]);
        }
        newArray->second.push_back(wantedValue((*filenames)[img]));
    }

    return newArray;
}

dataset * setupImages(std::string * path)
{
    ///---------------------------------- Png_byte** recuperation ---------------------------------------

    /// Initialization of the array containing all the images in the database in usable format
    auto images_array = new std::vector< std::vector< std::vector<double> > >();

    /// Initialization of the array containing all the images in the database in non-usable format
    auto images_png = new std::vector<png_bytepp>();

    /// Recovery of images one by one from their names/path
    static std::vector<char*> * fns;

    if(*path == "../../data/train/")
        fns = setDataSets(path);
    else
        fns = setDataSets(path);

    for(auto & fn : *fns)
        readPngFile(fn, images_png);


    ///--------------------------------- From png_byte** to double --------------------------------------


    /// Recover the number of images
    auto nbImg = static_cast<int>((*fns).size());

    /// Recover the image size
    int dim = 144;

    /// Convert for all images
    for(int img=0 ; img<nbImg ; img++)
    {
        /// Push-back an empty image
        images_array->push_back(* new std::vector< std::vector<double> >());

        /// Convert one image, row by row
        for(int i=0 ; i<dim ; i++)
        {
            /// Push-back an empty row
            (*images_array)[img].push_back(*(new std::vector<double>()));

            /// j is used to manage the RGB profile of the PNG data
            /// k is used to fill the 'images_array' that isn't able to manage RGB profile
            for(int j=0, k=0 ; j<dim && k<dim ; j++, k++)
            {
                /// Push-back the average of the RGB values in the 'image_array'
                (*images_array)[img][i].push_back((static_cast<double>((*images_png)[img][i][j])));
            }
        }
    }

    ///--------------------------------- Image's Size Adaptation ----------------------------------------

    auto images_array_rescale = new std::vector< std::vector< std::vector<double> > >();

    for(auto & i : *images_array)
    {
        auto resc = new ImageRescaler(&i, IMG_SIZE);
        images_array_rescale->push_back(*resc->rescale());
    }

    ///--------------------- Conversion of all images in a single array format --------------------------

    auto data = linearizeArray(images_array_rescale, fns);
    //auto data = linearizeArray(images_array, fns);

    ///-------------------------------------- Free memory -----------------------------------------------

    delete(images_png);
    delete(images_array);
    delete(fns);

    ///----------------------------------------- Return -------------------------------------------------

    return data;
}
