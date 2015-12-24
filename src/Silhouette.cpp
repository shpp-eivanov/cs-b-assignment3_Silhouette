/********************************************************************************************
* File: Silhouette.cpp
* ----------------------
* v.5 2015/12/24 - code reformatted
*
* Programm gets white-black image (GIF, PNG, JPEG, BMP),
* and discovers what object on it
* are actually could be human bodies silhouettes.
* Shows quantity of humans wich have
* been found.
********************************************************************************************/

#include <iostream>
#include <stdlib.h>
#include <Vector>
#include "queue.h"
#include "gbufferedimage.h"
#include "filelib.h"
#include "simpio.h"
#include "map.h"
#include <algorithm>
#include "console.h"

using namespace std;

/*------------------------------------------------------------------------------------------//
 * Declarations.
 * -----------------------------------------------------------------------------------------*/

/* Type: Pt
 * ----------
 * Combines coordinates of simple image pixel.
 * Pt - reduction of "point".*/
struct ImgPoint {
    int x;
    int y;
    bool operator ==(const ImgPoint & n1)
    {
        return (x == n1.x) && (y == n1.y);
    }
};

/* Global variables
 * -----------------
 * UNION_COLOR - integer of color for discovered
 * points union on the image */
int UNION_COLOR;
/* MINIMAL_SIZE - minimal valid size in pixels for
 * discovered points union on the image  */
int MINIMAL_SIZE;
/* NOT_HUMAN_OBJECT_COLOR - integer of color for
 * objects which aren't defined as human silhouette   */
int NOT_HUMAN_OBJECT_COLOR;

/* img - main program image  */
GBufferedImage* img;
/* Main image dimenisions. Are set global to
 * reduce programm calls */
int imgHeight;
int imgWidth;

/* Global constatnts
 * -------------------
 * Typical colors values in programm */
int const WHITE = 0xffffff;
int const GREEN = 0x00ff00;
int const BLUE = 0x0000ff;
int const BLACK = 0x000000;
int const PERIMETER_COLOR = WHITE;
/* FILTER_COLOR_LIMIT - some koef to make start image biColor */
int const FILTER_COLOR_LIMIT = 4000000;
/* PERIMETER_ERASE_KOEF - some koef to shrink humans bodies in effective way  */
int const PERIMETER_ERASE_KOEF = 30;//Useful range is 20 - 40;
/* MINIMAL_SIZE_KOEF - Size for garbage objects detection  */
int const MINIMAL_SIZE_KOEF = 1000;
/* Console size  */
int const CONSOLE_WIDTH = 700;
int const CONSOLE_HEIGHT = 400;

/* -----------------------------------------------------------------------------------------//
 * Implementation section
 * -----------------------------------------------------------------------------------------*/

/* Function: fileInput()
 * ----------------------------
 * Provides user file input.
 * Defines if this file exist in project directory */
string fileInput(string promptText) {
    /* Until user dosen't make valid input - ask him */
    string result = "";
    while (true) {
        result = getLine(promptText);
        if (fileExists(result)) {
            break;
        }else{
            cout << "Unfortunately your input is failed" << endl;
        }
    }
    return result;
}

/* Function: adjustMainWindow()
 * ----------------------------
 * Makes main program preparations - loads image, sets
 * main global variables */
void adjustMainWindow(GWindow& gw, string inputFile) {
    img = new GBufferedImage(800, 800, 0);
    img->load(inputFile);
    imgHeight = img->getHeight();
    imgWidth = img->getWidth();
    img->resize(imgWidth, imgHeight);

    gw.setSize(imgWidth, imgHeight);
    gw.setLocation(0, 0);

    string title = "Silhouette";
    setConsoleWindowTitle(title);
    setConsoleLocation(imgWidth, 0);
    setConsoleSize(CONSOLE_WIDTH, CONSOLE_HEIGHT);

    gw.add(img, 0, 0);

    MINIMAL_SIZE = ((imgWidth * imgHeight) / MINIMAL_SIZE_KOEF);//Size for garbage objects
    NOT_HUMAN_OBJECT_COLOR = BLUE;           //To show objects discovered as not human
}

/* Function: modifyImageToWhiteBlack()
 * ----------------------------------
 * Makes param image biColor. */
void modifyImageToWhiteBlack(GBufferedImage* img) {
    /* -----------------------------------------------------------*/
    cout << "       - FILTER IMAGE INTO BLACK-WHITE PIXELS" << endl;
    /* -----------------------------------------------------------*/
    int height = img->getHeight();
    int width = img->getWidth();
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int color = img->getRGB(col, row);
            if ((color < FILTER_COLOR_LIMIT)) {
                /* Each dark cell set as black */
                img->setRGB(col, row, BLACK);
            }else{
                /* Other cells set as white */
                img->setRGB(col, row, WHITE);
            }
        }
    }
}

/* Function: detectSingleObject()
 * ------------------------------
 * Returns vector of pixels, which are not white
 * on the main program image, and
 * which create single union object with
 * input param point. The function checks each pixel
 * around param pixel. If pixel is not white -
 * function adds it to  to result vector.
 *
 * @param row, col  pixel where first not white pixel is obtained  */
Vector<ImgPoint> detectSingleObject(int row, int col) {
    Vector<ImgPoint> result;
    int imgWidth2 = imgWidth;
    int imgeight2 = imgHeight;

    /* The first cell in result vector is param point */
    ImgPoint pt;
    pt.x = col;
    pt.y = row;
    Queue<ImgPoint> pointsQueue;
    pointsQueue.enqueue(pt);
    img->setRGB(col, row, UNION_COLOR);
    /* Finding of around cells process */
    while (!pointsQueue.isEmpty()) {
        pt = pointsQueue.dequeue();
        result.add(pt);
        /* Main around cells checking */
        int x = pt.x;
        int y = pt.y;
        if ((x > 0) && (x < (imgWidth2 - 1))) {
            if ((y > 0) && (y < (imgeight2 - 1))) {
                for (int i = x - 1; i <= x + 1; i++) {
                    for (int u = y - 1; u <= y + 1; u++) {
                        if ((i == x) && (u == y)) continue;
                        int cellColor = img->getRGB(i, u);
                        if ((cellColor != UNION_COLOR) && (cellColor != WHITE)) {
                            /* This pixel is new part for current union */
                            ImgPoint objPoint;
                            objPoint.x = i;
                            objPoint.y = u;
                            pointsQueue.enqueue(objPoint);
                            img->setRGB(i, u, UNION_COLOR);
                        }
                    }
                }
            }
        }
    }
    return result;
}

/* Function: detectImageObjects()
 * ------------------------------
 * Makes single iteration through program image, and
 * finds not white unions on it. Adds detected unions to
 * param vector.
 *
 * @param imageObjectsVector   All detected image objects storage vector   */
void detectImageObjects(Vector<Vector<ImgPoint>>& imageObjectsVector) {
    /* ------------------------------------------------*/
    cout << "       - ASSIGN PIXELS TO OBJECTS" << endl;
    /* ------------------------------------------------*/
    /* Main iteration through image */
    for (int row = 0; row < (imgHeight); row++) {
        for (int col = 0; col < (imgWidth); col++) {
            int cellColor = img->getRGB(col, row);

            if ((cellColor != UNION_COLOR) && (cellColor != WHITE)) {
                /* If the first pixel of possible object is detected  - check all her
                 * around pixels  */
                Vector<ImgPoint> imgObject = detectSingleObject(row, col);
                if (imgObject.size() > MINIMAL_SIZE) {
                    imageObjectsVector.add(imgObject);
                }else{
                    /* Erase the smallest objects */
                    for (int u = 0; u < imgObject.size(); u++) {
                        ImgPoint i = imgObject[u];
                        img->setRGB(i.x, i.y, WHITE);
                    }
                }
            }
        }
    }
    /* -------------------------------------------------*/
    cout << "       - UNIONS BEFORE HUMAN-PROPORTIONS CHECKING = "
         << imageObjectsVector.size() << endl;
    /* -------------------------------------------------*/
}

/* Function: loadValuesToMaxes()
 * -----------------------------
 * Finds edges of the param object, and store their values into
 * param references.
 *
 * @param min, max   at this references obtained edges values will be load
 * @param objectVec  object to process  */
void loadValuesToMaxes(int& min_X, int & max_X, int& min_Y, int& max_Y,
                       Vector<ImgPoint>& objectVec) {
    /* Initiates params values by some point values */
    for (int u = 0; u < objectVec.size(); u++) {
        ImgPoint i = objectVec[u];
        min_X = i.x;
        min_X = i.x;
        min_Y = i.y;
        max_Y = i.y;
        break;
    }
    /* Finds max and min edges values through the object */
    for (int u = 0; u < objectVec.size(); u++) {
        ImgPoint i = objectVec[u];
        if (i.x < min_X) {
            min_X = i.x;
        }
        if (i.x > max_X) {
            max_X = i.x;
        }
        if (i.y < min_Y) {
            min_Y = i.y;
        }
        if (i.y > max_Y) {
            max_Y = i.y;
        }
    }
}

/* Function: getObjectMiddleWidth()
 * -------------------------------
 * Returns width of the param object on the level of his middle_Y
 * coordinate ("waistline")
 *
 * @param middle_Y   average middle Y coordinate of the param object
 * @param objectVec  object to process  */
int getObjectMiddleWidth(int middle_Y, Vector<ImgPoint> objectVec) {
    int result = 0;
    int middleMin_X = 0;
    int middleMax_X = 0;

    /* Initiates max values by first obtained point values */
    for (int u = 0; u < objectVec.size(); u++) {
        ImgPoint i = objectVec[u];
        if (i.y == middle_Y) {
            middleMin_X = i.x;
            middleMax_X = i.x;
            break;
        }
    }

    /* Finds edges on object the middle Y level */
    for (int u = 0; u < objectVec.size(); u++) {
        ImgPoint i = objectVec[u];
        if (i.y == middle_Y) {
            if (i.x < middleMin_X) {
                middleMin_X = i.x;
            }
            if (i.x > middleMax_X) {
                middleMax_X = i.x;
            }
        }
    }

    result = middleMax_X - middleMin_X;
    return result;
}

/* Function: repaintObjectPerimeterColor()
 * ---------------------------------------
 * Repaint object perimeter pixels in PERIMETR_COLOR
 * for 1 time due to simple condition: if there are other
 * colors cells around current cell - it's perimeter cell.
 *
 * @param objectVec  object to process  */
void repaintObjectPerimeterColor(Vector<ImgPoint>& objectVec) {
    Map<int, ImgPoint> perimeterMap;

    for (int k = 0; k < objectVec.size(); k++) {
        ImgPoint point = objectVec[k];
        int x = point.x;
        int y = point.y;

        if ((x > 0) && (x < (img->getWidth() - 1))) {
            if ((y > 0) && (y < (img->getHeight() - 1))) {
                for (int i = x - 1; i <= x + 1; i++) {
                    for (int u = y - 1; u <= y + 1; u++) {
                        if ((i == x) && (u == y)) continue;

                        if (img->getRGB(i, u) != UNION_COLOR) {
                            if (!perimeterMap.containsKey(k)) {
                                perimeterMap.add(k, point);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    /* To prepare perimeter points removing
     * from main objectVec put their objectVec indexes
     * into keysToRemove vector */
    Vector<int> keysToRemove = perimeterMap.keys();
    /* Sort indexes - then we could make
     * removement from main objectVect correctly, from end
     * to begin */
    sort(keysToRemove.begin(), keysToRemove.end());

    /* Repaint perimetr cells, and remove them from objectVec */
    for (int i = (keysToRemove.size() - 1); i >= 0; i--) {
        int removeFromObjIndex = keysToRemove[i];
        ImgPoint pointToRemove = perimeterMap[removeFromObjIndex];

        img->setRGB(pointToRemove.x, pointToRemove.y, PERIMETER_COLOR);

        objectVec.remove(removeFromObjIndex);
    }
}

/* Function: eraseAllObjectsPerimeters()
 * -------------------------------------
 * For every object from input param vector chooses allowable
 * level of object  shrinking and calls repaintPerimetrColor()
 * to repaint object perimetr into another color - white
 * for example.
 *
 * @param imageObjectsVector  All detected image objects vector  */
void eraseAllObjectsPerimeters(Vector<Vector<ImgPoint>>& imageObjectsVector) {
    /* -------------------------------------------------*/
    cout << "       - SHRINK OBJECTS PERIMETERS" << endl;
    /* -------------------------------------------------*/
    for (int i = 0; i < imageObjectsVector.size(); i++) {
        Vector<ImgPoint> objectVec = imageObjectsVector[i];

        int min_X = 0;
        int max_X = 0;
        int min_Y = 0;
        int max_Y = 0;
        loadValuesToMaxes(min_X, max_X, min_Y, max_Y, objectVec);

        int objectHeight = max_Y - min_Y;
        /* Calculate count of shrinks for this object due to some human body height  */
        int perimeterErasesQty = (objectHeight / PERIMETER_ERASE_KOEF) - 1;//absolutely empirical formula

        for (int u = 0; u < perimeterErasesQty; u++) {
            repaintObjectPerimeterColor(objectVec);
        }
    }
}

/* Function: silhouettesCounting()
 * -------------------------------
 * Returns quantity of objects, which could be
 * human silhouettes on the image. Belive that object is human
 * if it's waistline isn't bigger then half of it's height.
 * Too "low" objects are considered as garbage and erased.
 * Supose that all humans are pictured in the same scale.
 *
 * @param imageObjectsVector  All detected image objects vector  */
int silhouettesCounting(Vector<Vector<ImgPoint>> imageObjectsVector) {
    int silhouettesQty = 0;//Total quantity of humans on the image
    int maxImageHeight = 0;//The highest object height

    for (int i = 0; i < imageObjectsVector.size(); i++) {
        Vector<ImgPoint> object = imageObjectsVector[i];

        int min_X, max_X, min_Y, max_Y;
        min_X = max_X = min_Y = max_Y = 0;
        loadValuesToMaxes(min_X, max_X, min_Y, max_Y, object);

        int objectHeight = max_Y - min_Y;//Current object height

        if (maxImageHeight < objectHeight) {
            maxImageHeight = objectHeight;//Update highest object height
        }

        int middle_Y = min_Y + (objectHeight / 2);//Middle of the "human body"
        /* Find out length of waistline of the "human body" */
        int middleWidth = getObjectMiddleWidth(middle_Y, object);

        /* Main "human" proportion evaluation:
         * - if waistline isn't bigger then half of it's height - belive this is human
         * - supose, that minimal human silhouette wouldn't be 5 times lower then
         *   highest object, else - it is erased as garbage */
        int objectMinimalHeight = maxImageHeight / 5;

        if (((objectHeight / 2) > middleWidth) && (objectHeight > objectMinimalHeight)) {
            silhouettesQty++;//Apreciate current object as "human body silhouette"
        }else{//If object haven't matched propotions conditions - it's erased from image
            for (int u = 0; u < object.size(); u++) {
                ImgPoint i = object[u];
                /* Repaint no human objects into garbageColor */
                img->setRGB(i.x, i.y, NOT_HUMAN_OBJECT_COLOR);
            }
        }
    }
    /* -------------------------------------------------*/
    cout << "       - HUMANS QUANTITY DUE TO PROPORTIONS = "
         << silhouettesQty << endl;
    /* -------------------------------------------------*/
    return silhouettesQty;
}


int main() {
    /* --------------------------------------------------------------------------*/
    string inputFile = "girls.jpg";//Enter your image file here
    /* --------------------------------------------------------------------------*/
    //string inputFile = fileInput("Enter your image file here: ");

    GWindow gw;
    adjustMainWindow(gw, inputFile);//Main window and program settings

    cout << "PROCESSING..." << endl;
    cout << "==========================================================" << endl;
    modifyImageToWhiteBlack(img);

    UNION_COLOR = GREEN;//To show union detection processing
    Vector<Vector<ImgPoint>> imageObjects1;
    detectImageObjects(imageObjects1);
    int qty1 = silhouettesCounting(imageObjects1);

    eraseAllObjectsPerimeters(imageObjects1);

    UNION_COLOR = BLACK;//To return black-white picture from function
    Vector<Vector<ImgPoint>> imageObjects2;
    detectImageObjects(imageObjects2);
    int qty2 = silhouettesCounting(imageObjects2);

    int averageQty = (qty1 + qty2) / 2;

    cout << "==========================================================" << endl;
    cout << "PROGRAM IS FINISHED. AVERAGE HUMANS QUANTITY IS: " << averageQty << endl;

    return 0;
}
