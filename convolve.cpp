/*
 * Program description: Creates an alphamask of an image
 *
 * Author: Austin Brennan - awbrenn@g.clemson.edu
 *
 * Date: October 2, 2014
 *
 * Other information: Check README for more information
 *
 */

#include <cstdlib>
#include <iostream>
#include <vector>
#include <OpenImageIO/imageio.h>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;
OIIO_NAMESPACE_USING

// Struct Declaration
struct pixel {
    float r, g, b, a;
};

// Global Variables
int IMAGE_HEIGHT;
int IMAGE_WIDTH;
pixel **PIXMAP;
char *OUTPUT_FILE = NULL;
int FILTER_SIZE;
float **FILTER;

class Image {
public:
    int height;
    int width;
    pixel ** pixmap;

    Image(int, int);
};

Image::Image(int x, int y) {
    width = x;
    height = y;
    pixmap = new pixel*[height];
    pixmap[0] = new pixel[width * height];

    for (int i = 1; i < height; i++)
        pixmap[i] = pixmap[i - 1] + width;
}

/* Handles errors
 * input	- message is printed to stdout, if kill is true end program
 * output	- None
 * side effect - prints error message and kills program if kill flag is set
 */
void handleError (string message, bool kill) {
    cout << "ERROR: " << message << "\n";
    if (kill)
        exit(0);
}

/* Converts pixels from vector to pixel pointers
 * input		- vector of pixels, number of channels
 * output		- None
 * side effect	- saves pixel data from vector to PIXMAP
 */
Image convertVectorToImage (vector<unsigned char> vector_pixels, int channels) {
    Image image(IMAGE_WIDTH, IMAGE_HEIGHT);

    int i = 0;
    if (channels == 3) {
        for (int row = image.height-1; row >= 0; row--)
            for (int col = 0; col < image.width; col++) {
                image.pixmap[row][col].r = (float) vector_pixels[i++] / 255.0;
                image.pixmap[row][col].g = (float) vector_pixels[i++] / 255.0;
                image.pixmap[row][col].b = (float) vector_pixels[i++] / 255.0;
                image.pixmap[row][col].a = 1.0;
            }
    }
    else if (channels == 4) {
        for (int row = image.height-1; row >= 0; row--)
            for (int col = 0; col < image.width; col++) {
                image.pixmap[row][col].r = (float) vector_pixels[i++] / 255.0;
                image.pixmap[row][col].g = (float) vector_pixels[i++] / 255.0;
                image.pixmap[row][col].b = (float) vector_pixels[i++] / 255.0;
                image.pixmap[row][col].a = (float) vector_pixels[i++] / 255.0;
            }
    }

    else
        handleError ("Could not convert image.. sorry", 1);

    return image;
}

pixel ** flipImageVertical(pixel **pixmap_vertical_flip) {
    for (int row = IMAGE_HEIGHT-1; row >= 0; row--)
        for (int col = 0; col < IMAGE_WIDTH; col++) {
            pixmap_vertical_flip[(IMAGE_HEIGHT-1)-row][col] = PIXMAP[row][col];
        }

    return pixmap_vertical_flip;
}

/* Reads image specified in argv[1]
 * input		- the input file name
 * output		- None
 */
Image readImage (string filename) {
    ImageInput *in = ImageInput::open(filename);
    if (!in)
        handleError("Could not open input file", true);
    const ImageSpec &spec = in->spec();
    IMAGE_WIDTH = spec.width;
    IMAGE_HEIGHT = spec.height;
    int channels = spec.nchannels;
    if(channels < 3 || channels > 4)
        handleError("Application supports 3 or 4 channel images only", 1);
    vector<unsigned char> pixels (IMAGE_WIDTH*IMAGE_HEIGHT*channels);
    in->read_image (TypeDesc::UINT8, &pixels[0]);
    in->close ();
    delete in;


    return convertVectorToImage(pixels, channels);
}

/* Write image to specified file
 * input		- pixel array, width of display window, height of display window
 * output		- None
 * side effect	- writes image to a file
 */
void writeImage(unsigned char *glut_display_map, int window_width, int window_height) {
    const char *filename = OUTPUT_FILE;
    const int xres = window_width, yres = window_height;
    const int channels = 4; // RGBA
    int scanlinesize = xres * channels;
    ImageOutput *out = ImageOutput::create (filename);
    if (! out) {
        handleError("Could not create output file", false);
        return;
    }
    ImageSpec spec (xres, yres, channels, TypeDesc::UINT8);
    out->open (filename, spec);
    out->write_image (TypeDesc::UINT8, glut_display_map+(window_height-1)*scanlinesize, AutoStride, -scanlinesize, AutoStride);
    out->close ();
    delete out;
    cout << "SUCCESS: Image successfully written to " << OUTPUT_FILE << "\n";
}

/* Draw Image to opengl display
 * input		- None
 * output		- None
 * side effect	- draws image to opengl display window
 */
void drawImage() {
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glRasterPos2i(0,0);
    glDrawPixels(IMAGE_WIDTH, IMAGE_HEIGHT, GL_RGBA, GL_FLOAT, PIXMAP[0]);
    glFlush();
}

Image convolveImage(Image copy_image) {
    Image convolved_image(IMAGE_WIDTH, IMAGE_HEIGHT);

    for (int row = 0; row < copy_image.height; row++)
        for (int col = 0; col < copy_image.width; col++) {
            convolved_image.pixmap[row][col] = copy_image.pixmap[row][col];
        }

    cout << "got here" << endl;
    delete copy_image.pixmap;
    return convolved_image;
}

/* Key press handler
 * input	- Handled by opengl, because this is a callback function.
 * output	- None
 */
void handleKey(unsigned char key, int x, int y) {

    if (key == 'w') {
        if(OUTPUT_FILE != NULL) {
            int window_width = glutGet(GLUT_WINDOW_WIDTH), window_height = glutGet(GLUT_WINDOW_HEIGHT);
            unsigned char glut_display_map[window_width*window_height*4];
            glReadPixels(0,0, window_width, window_height, GL_RGBA, GL_UNSIGNED_BYTE, glut_display_map);
            writeImage(glut_display_map, window_width, window_height);
        }
        else
            handleError("Cannot write to file. Specify filename in third argument to write to file.", false);
    }
    else if (key == 'q' || key == 'Q') {
        cout << "\nProgram Terminated." << endl;
        exit(0);
    }
    else if (key == 'c') {
        Image copy_image(IMAGE_WIDTH, IMAGE_HEIGHT);

        delete copy_image.pixmap;
        copy_image.pixmap = PIXMAP;


        Image convolved_image = convolveImage(copy_image);
        PIXMAP = convolved_image.pixmap;
        drawImage();
    }
}

/* Initialize opengl
 * input	- command line arguments
 * output	- none
 */
void openGlInit(int argc, char* argv[]) {
    // start up the glut utilities
    glutInit(&argc, argv);

    // create the graphics window, giving width, height, and title text
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowSize(IMAGE_WIDTH, IMAGE_HEIGHT);
    glutCreateWindow("A Display for an Image");

    // set up the callback routines to be called when glutMainLoop() detects
    // an event
    glutDisplayFunc(drawImage);		  		// display callback
    glutKeyboardFunc(handleKey);	  		// keyboard callback
    // glutMouseFunc(handleClick);		// click callback

    // define the drawing coordinate system on the viewport
    // lower left is (0, 0), upper right is (WIDTH, HEIGHT)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, IMAGE_WIDTH, 0, IMAGE_HEIGHT);

    // specify window clear (background) color to be opaque white
    glClearColor(1, 1, 1, 0);

    // Routine that loops forever looking for events. It calls the registered
    // callback routine to handle each event that is detected
    glutMainLoop();
}

void readFilter(char *filter_filename) {
    FILE * pFile;
    pFile = fopen (filter_filename, "r");
    int filter_size;
    float filter_value;
    fscanf(pFile, "%d", &filter_size);
    FILTER_SIZE = filter_size;
    FILTER = new float*[FILTER_SIZE];
    FILTER[0] = new float[FILTER_SIZE*FILTER_SIZE];

    for (int i = 1; i < FILTER_SIZE; i++)
        FILTER[i] = FILTER[i - 1] + FILTER_SIZE;

    for(int row = 0; row < FILTER_SIZE; row++)
        for(int col = 0; col < FILTER_SIZE; col++) {
            fscanf(pFile, "%f", &filter_value);
            FILTER[row][col] = filter_value;
        }

    fclose(pFile);
}

float maximum(float a, float b) {
    if (a < b)
        return b;
    else if (a > b)
        return a;

    return a;
}

void normalizeFilter() {
    float scale_factor, filter_value, sum_of_negative_values = 0, sum_of_posotive_values = 0;

    for (int row = 0; row < FILTER_SIZE; row++)
        for (int col = 0; col < FILTER_SIZE; col++) {
            filter_value = FILTER[row][col];
            if (filter_value < 0)
                sum_of_negative_values += -1.0 * filter_value; // multiply by -1.0 for absolute value
            else if (filter_value > 0)
                sum_of_posotive_values += filter_value;
        }
    scale_factor = maximum(sum_of_posotive_values, sum_of_negative_values);
    if (scale_factor != 0)
        scale_factor = 1.0 / scale_factor;
    else
        scale_factor = 1.0;

    for (int row = 0; row < FILTER_SIZE; row++)
        for (int col = 0; col < FILTER_SIZE; col++) {
            FILTER[row][col] = scale_factor * FILTER[row][col];
        }
}

void flipFilterXandY() {
    float **temp_filter;

    temp_filter = new float*[FILTER_SIZE];
    temp_filter[0] = new float[FILTER_SIZE*FILTER_SIZE];

    for (int i = 1; i < FILTER_SIZE; i++)
        temp_filter[i] = temp_filter[i - 1] + FILTER_SIZE;

    for (int row = 0; row < FILTER_SIZE; row++)
        for (int col = 0; col < FILTER_SIZE; col++) {
            temp_filter[row][col] = FILTER[(FILTER_SIZE-1)-row][(FILTER_SIZE-1)-col];
        }

    for (int row = 0; row < FILTER_SIZE; row++)
        for (int col = 0; col < FILTER_SIZE; col++) {
            FILTER[row][col] = temp_filter[row][col];
        }
    
    delete temp_filter;
}

int main(int argc, char *argv[]) {
    if (argc != 3 and argc != 4)
        handleError("Proper use:\n$> filt filter.filt input.img\n"
                "$> filt filter.filt input.img output.img", 1);
    if (argc == 4) // specified output file
        OUTPUT_FILE = argv[3];

    readFilter(argv[1]);
    normalizeFilter();
    flipFilterXandY();

    for (int row = 0; row < FILTER_SIZE; row++) {
        for (int col = 0; col < FILTER_SIZE; col++) {
            cout << FILTER[row][col] << " ";
        }
        cout << endl;
    }
//    Image original_image = readImage(argv[2]);
//    PIXMAP = original_image.pixmap;
//    openGlInit(argc, argv);
}