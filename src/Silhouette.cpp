/********************************************************************************************
 * File: Silhouette.cpp
 * ----------------------
 * v.1 2015/11/05
 * Programm gets white-black image (GIF, PNG, JPEG, BMP),
 * and discovers what object on it
 * are actually could be human bodies silhouettes.
 * Shows quantity of humans wich have
 * been found.
 ********************************************************************************************/

#include <iostream>
#include <stdlib.h>
#include "queue.h"
#include <Vector>
#include "gbufferedimage.h"

using namespace std;

struct Pt{
    int x;
    int y;
    bool operator==(const Pt & n1){
             return (x == n1.x) && (y == n1.y);
    }
};

int unionColor;
int weakObject;
int garbageColor;

int const WHITE = 0xffffff;
int const GREEN = 0x00ff00;
int const BLUE = 0x0000ff;
int const BLACK = 0x000000;
int const PERIMETR_COLOR = WHITE;

/* Some koef to make start image biColor */
int const FILTER_COLOR_LIMIT = 4000000;
/* Some koef to shrink humans bodies   */
int const SHRINK_KOEF = 30;

GBufferedImage* img;

/* Make biColor image */
void filterImage(GBufferedImage* img){
    /* -----------------------------------------------------------*/
    cout << "       - FILTER IMAGE INTO BLACK-WHITE PIXELS" << endl;
    /* -----------------------------------------------------------*/
    int height = img->getHeight();
    int width = img->getWidth();
    for(int row = 0; row < (height); row++){
        for(int col = 0; col < (width); col++){
            int color = img->getRGB(col, row);
            if((color < FILTER_COLOR_LIMIT)){
            /* Each dark cell set as black */
                img->setRGB(col, row, 0x0);
            }else{
            /* Other cells set as white */
                img->setRGB(col, row, WHITE);
            }
        }
    }
}

/* Checks if this vector contain such value */
bool isVectorContains(Pt value, Vector<Pt>& vec){
    bool contains = false;
    for(Pt vecElem: vec){
        if(vecElem == value){
            contains = true;
            break;
        }
    }
    return contains;
}

void removeFromVector(Pt& cell, Vector<Pt>& vec){
    for(int i = 0; i < vec.size(); i++){
        if (vec[i] == cell){
            vec.remove(i);
            break;
        }
    }
}

/* Adds current cell and all around cells to object array */
Vector<Pt> createCellsArray(int row, int col){
    Vector<Pt> result;
    /* Add first cell */
    Pt pt;
    pt.x = col;
    pt.y = row;
    Queue<Pt> pointsQueue;
    Vector<Pt>queueList;
    pointsQueue.enqueue(pt);
    queueList.add(pt);
    /* Around cells queue process */
    while(!pointsQueue.isEmpty()){
        pt = pointsQueue.dequeue();
        removeFromVector(pt, queueList);
        if(!isVectorContains(pt, result)){
            result.add(pt);          
            img->setRGB(pt.x, pt.y, unionColor);

            int imgWidth = img->getWidth();
            int imgHeight = img->getHeight();
            int x = pt.x;
            int y = pt.y;
            if((x > 0) && (x < (imgWidth - 1))){
                if((y > 0) && (y < (imgHeight - 1))){
                    for(int i = x-1; i <= x+1; i++){
                        for(int u = y-1; u <= y+1; u++){
                            int cellColor = img->getRGB(i, u);
                            if((cellColor != unionColor) && (cellColor != WHITE)){
                                Pt objPoint;
                                objPoint.x = i;
                                objPoint.y = u;
                                if(!isVectorContains(objPoint, result)){
                                    if(!isVectorContains(objPoint, queueList)){
                                        pointsQueue.enqueue(objPoint);
                                        queueList.add(objPoint);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

/* Make first iteration through picture and put cells to
 * some objects (unions) vectors */
void assignCells (GBufferedImage* img, Vector<Vector<Pt>>& unionsTable){
    /* ------------------------------------------------*/
    cout << "       - ASSIGN PIXELS TO OBJECTS" << endl;
    /* ------------------------------------------------*/
    int imgWidth = img->getWidth();
    int imgHeight = img->getHeight();
    /* Main pass through image */
    for(int row = 0; row < (imgHeight); row++){
        for(int col = 0; col < (imgWidth); col++){
            int cellColor = img->getRGB(col, row);
            /* Isn't white or yet object colored cell */
            if((cellColor != unionColor) && (cellColor != WHITE)){
                 Vector<Pt> vec = createCellsArray(row, col);
                 if(vec.size() > weakObject){
                     unionsTable.add(vec);
                 }else{
                     /* Erase object the smallest objects */
                     for(Pt i: vec){
                         img->setRGB(i.x, i.y, WHITE);
                     }
                 }
            }
        }
    }
    /* -------------------------------------------------*/
    cout << "       - UNIONS BEFORE HUMAN-PROPORTIONS CHECKING = "
         << unionsTable.size() << endl;
    /* -------------------------------------------------*/
}

/* Finds edges points of the object */
void findMaxes(int& min_X, int& max_X, int& min_Y, int& max_Y, Vector<Pt>& objectVec){
    /* initiate max values by some point */
    for(Pt i: objectVec){
        min_X = i.x;
        min_X = i.x;
        min_Y = i.y;
        max_Y = i.y;
        break;
    }
    /* Find max and min values */
    for(Pt i: objectVec){
        if(i.x < min_X){
            min_X = i.x;
        }
        if(i.x > max_X){
            max_X = i.x;
        }
        if(i.y < min_Y){
            min_Y = i.y;
        }
        if(i.y > max_Y){
            max_Y = i.y;
        }
    }
}

/* Counts real middle width of the object */
int countMiddleWidth(int middle_Y, Vector<Pt> objectVec){
    int middleWidth = 0;
    int middleMin_X = 0;
    int middleMax_X = 0;
    /* Initiates max values by some point */
    for(Pt i: objectVec){
        if(i.y == middle_Y){
            middleMin_X = i.x;
            middleMax_X = i.x;
            break;
        }
    }
    /* Finds edges on the middle Y level */
    for(Pt i: objectVec){
        if(i.y == middle_Y){
            if(i.x < middleMin_X){
                middleMin_X = i.x;
            }
            if(i.x > middleMax_X){
                middleMax_X = i.x;
            }
        }
    }
    middleWidth = middleMax_X - middleMin_X;
    return middleWidth;
}

/* Repaint object perimeter for 1 time due to simple condition -
 * if there are other colors cells around current cell - it's perimeter cell */
void repaintPerimetrColor(Vector<Pt>& objectVec){
    Vector<Pt> perimeter;
    for(Pt point: objectVec){
        int x = point.x;
        int y = point.y;
        int pointColor = img->getRGB(x, y);
        if((x > 0) && (x < (img->getWidth() - 1))){
            if((y > 0) && (y < (img->getHeight() - 1))){
                for(int i = x-1; i <= x+1; i++){
                    for(int u = y-1; u <= y+1; u++){                       
                        if(img->getRGB(i, u) != pointColor){
                            perimeter.add(point);
                            break;
                        }
                    }
                }
            }
        }
    }
    /* Repaint all perimetr cells */
    for(Pt i: perimeter){
        img->setRGB(i.x, i.y, PERIMETR_COLOR);
        removeFromVector(i, objectVec);
    }
}

/* Shrink object perimetr pixels due to some human body proportions.
 * Decide level of shrinking before */
void shrinkObjects(Vector<Vector<Pt>>& unionsTable){
    /* -------------------------------------------------*/
    cout << "       - SHRINK OBJECTS PERIMETERS" << endl;
    /* -------------------------------------------------*/
    for(int i = 0; i < unionsTable.size(); i++){
        Vector<Pt> objectVec = unionsTable[i];
        int min_X = 0;
        int max_X = 0;
        int min_Y = 0;
        int max_Y = 0;
        findMaxes(min_X, max_X, min_Y, max_Y, objectVec);
        int objectHeight = max_Y - min_Y;
        /* Calculate count of shrink for this object due to some
         * human body koefs   */
        int perimetrShrinks = (objectHeight/SHRINK_KOEF) - 1;//
        for(int u = 0; u < perimetrShrinks; u++){
            repaintPerimetrColor(objectVec);
        }
    }
}

/* Count humans through the objects and mark no humans objects  */
int countHumans(Vector<Vector<Pt>> unionsTable){
    int result = 0;
    int maxImageHeight = 0;
    for(int i = 0; i <unionsTable.size(); i++){
        Vector<Pt> objectVec = unionsTable[i];
        int min_X = 0;
        int max_X = 0;
        int min_Y = 0;
        int max_Y = 0;
        findMaxes(min_X, max_X, min_Y, max_Y, objectVec);
        int objectHeight = max_Y - min_Y;
        if(maxImageHeight < objectHeight){
            maxImageHeight = objectHeight;//The highest object height
        }
        int middle_Y = min_Y + (objectHeight/2);
        int middleWidth = countMiddleWidth(middle_Y, objectVec);
        /* Main "human" proportion process */
        if((objectHeight > (2*middleWidth)) && (objectHeight > (maxImageHeight/5))){
                result++;
        }else{
            for(Pt i: objectVec){
                /* Set no human objects as garbageColor */
                img->setRGB(i.x, i.y, garbageColor);
            }
        }
    }
    /* -------------------------------------------------*/
    cout << "       - HUMANS QUANTITY DUE TO PROPORTIONS = "
         << result << endl;
    /* -------------------------------------------------*/
    return result;
}



int main(){
    GWindow gw;
    img = new GBufferedImage(800,800, 0);
    /* Enter your image file */
    img->load("picture-large-girls-silhouettes.jpg");
    int imgHeight = img->getHeight();
    int imgWidth = img->getWidth();
    img->resize(imgWidth, imgHeight);
    gw.setSize(imgWidth, imgHeight);
    gw.add(img, 0, 0);

    cout << "PROCESSING..." << endl;
    cout << "==========================================================" << endl;
    filterImage(img);

    unionColor = GREEN;
    weakObject = ((imgWidth * imgHeight)/1000);//Size for garbage objects
    garbageColor = BLUE;//To show objects discovered as not human
    Vector<Vector<Pt>> unionsTable;
    assignCells(img, unionsTable);
    /* Supose, that minimal human silhouette wouldn't be 5 times lower then
     * then highest object */
    int humans1 = countHumans(unionsTable);

    shrinkObjects(unionsTable);

    unionColor = BLACK;//To return black-white picture from function
    weakObject = weakObject;
    garbageColor = BLUE;//To show objects discovered as not human
    Vector<Vector<Pt>> unionsTable2;
    assignCells(img, unionsTable2);
    int humans2 = countHumans(unionsTable2);

    int averageQty = (humans1 + humans2) / 2;
    cout << "==========================================================" << endl;
    cout << "PROGRAM IS FINISHED. AVERAGE HUMANS QUANTITY IS: " << averageQty << endl;

    return 0;
}
