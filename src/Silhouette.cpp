#include <iostream>
#include <stdlib.h>
#include "queue.h"
#include "myVector.h"
#include "gbufferedimage.h"

using namespace std;

//Дочитать о const и оформлении шаблонных классов
//Добавить remove по value а не по индексу


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


void removeFromVector(Pt& cell, MyVector<Pt>& vec){
    for(int i = 0; i < vec.size(); i++){
        Pt elem = vec.get(i);
        if (elem == cell){
            vec.remove(i);
            break;
        }
    }
}

/* Adds current cell and all around cells to object array */
MyVector<Pt> createCellsArray(int row, int col){
    MyVector<Pt> result;
    /* Add first cell */
    Pt pt;
    pt.x = col;
    pt.y = row;
    Queue<Pt> pointsQueue;
    MyVector<Pt>queueList;
    pointsQueue.enqueue(pt);
    queueList.add(pt);
    /* Around cells queue process */
    while(!pointsQueue.isEmpty()){
        pt = pointsQueue.dequeue();
        queueList.removeValue(pt);
        if(!result.isContains(pt)){
            result.add(pt);          
            img->setRGB(pt.x, pt.y, unionColor);
            //addNeighboursToQueue(pt, pointsQueue, queueList, result);
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
                                if(!result.isContains(objPoint)){
                                    if(!queueList.isContains(objPoint)){
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
void assignCells (GBufferedImage* img, MyVector<MyVector<Pt>>& unionsTable){
    int imgWidth = img->getWidth();
    int imgHeight = img->getHeight();
    /* Main pass through image */
    for(int row = 0; row < (imgHeight); row++){
        for(int col = 0; col < (imgWidth); col++){
            int cellColor = img->getRGB(col, row);
            /* Isn't white or yet object colored cell */
            if((cellColor != unionColor) && (cellColor != WHITE)){
                 MyVector<Pt> vec = createCellsArray(row, col);
                 if(vec.size() > weakObject){
                     unionsTable.add(vec);
                 }else{
                     /* Erase the smallest objects */
                     for(int u = 0; u < vec.size(); u++){
                         Pt i = vec.get(u);
                         img->setRGB(i.x, i.y, WHITE);
                     }
                 }
            }
        }
    }
}

/* Finds edges points of the object */
void findMaxes(int& min_X, int& max_X, int& min_Y, int& max_Y, MyVector<Pt>& objectVec){
    /* initiate max values by some point */
    for(int u = 0; u < objectVec.size(); u++){
        Pt i = objectVec.get(u);
        min_X = i.x;
        min_X = i.x;
        min_Y = i.y;
        max_Y = i.y;
        break;
    }
    /* Find max and min values */
    for(int u = 0; u < objectVec.size(); u++){
        Pt i = objectVec.get(u);
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
int countMiddleWidth(int middle_Y, MyVector<Pt> objectVec){
    int middleWidth = 0;
    int middleMin_X = 0;
    int middleMax_X = 0;
    /* Initiates max values by some point */
    for(int u = 0; u < objectVec.size(); u++){
        Pt i = objectVec.get(u);
        if(i.y == middle_Y){
            middleMin_X = i.x;
            middleMax_X = i.x;
            break;
        }
    }
    /* Finds edges on the middle Y level */
    for(int u = 0; u < objectVec.size(); u++){
        Pt i = objectVec.get(u);
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
void repaintPerimetrColor(MyVector<Pt>& objectVec){
    MyVector<Pt> perimeter;
    for(int u = 0; u < objectVec.size(); u++){
        Pt point = objectVec.get(u);
        int x = point.x;
        int y = point.y;
        int pointColor = img->getRGB(x, y);
        if((x > 0) && (x < (img->getWidth() - 1))){
            if((y > 0) && (y < (img->getHeight() - 1))){
                for(int i = x-1; i <= x+1; i++){
                    for(int u = y-1; u <= y+1; u++){                       
                        if(img->getRGB(i, u) != pointColor){
                            if(!perimeter.isContains(point)){
                                    perimeter.add(point);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    cout << objectVec.size() << " " << perimeter.size()<< endl;
    /* Repaint all perimetr cells */
    for(int u = 0; u < perimeter.size(); u++){
        Pt i = perimeter.get(u);
        cout << i.x << " " << i.y << endl;
        img->setRGB(i.x, i.y, PERIMETR_COLOR);
        objectVec.removeValue(i);
    }
}

/* Shrink object perimetr pixels due to some human body proportions.
 * Decide level of shrinking before */
void shrinkObjects(MyVector<MyVector<Pt>>& unionsTable){
    for(int i = 0; i < unionsTable.size(); i++){
        MyVector<Pt> objectVec = unionsTable.get(i);
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
int countHumans(MyVector<MyVector<Pt>> unionsTable){
    int result = 0;
    int maxImageHeight = 0;
    for(int i = 0; i <unionsTable.size(); i++){
        MyVector<Pt> objectVec = unionsTable.get(i);
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
            for(int u = 0; u < objectVec.size(); u++){
                Pt i = objectVec.get(u);
                /* Set no human objects as garbageColor */
                img->setRGB(i.x, i.y, garbageColor);
            }
        }
    }
    return result;
}



int main(){
    GWindow gw;
    img = new GBufferedImage(800,800, 0);
    /* Enter your image file */
    img->load("girls.jpg");
    int imgHeight = img->getHeight();
    int imgWidth = img->getWidth();
    img->resize(imgWidth, imgHeight);
    gw.setSize(imgWidth, imgHeight);
    gw.add(img, 0, 0);

    cout << "*****FILTER IMAGE INTO BLACK-WHITE PIXELS*******" << endl;
    filterImage(img);

    cout << "********FIRST ASSIGN CELLS TO OBJECTS***********" << endl;
    unionColor = GREEN;
    weakObject = ((imgWidth * imgHeight)/1000);//Size for garbage objects
    garbageColor = BLUE;//To show objects discovered as not human
    MyVector<MyVector<Pt>> unionsTable;
    assignCells(img, unionsTable);

    cout << "UNIONS BEFORE HUMAN-PROPORTIONS CHECKING = " << unionsTable.size() << endl;

    /* Supose, that minimal human silhouette wouldn't be 5 times lower then
     * then highest object */
    int humans1 = countHumans(unionsTable);
    cout << "HUMANS QUANTITY DUE TO PROPORTIONS = " << humans1 << endl;


    cout << "**********SHRINK OBJECT PERIMETR****************" << endl;
    shrinkObjects(unionsTable);


    cout << "*******SECOND ASSIGN CELLS TO OBJECTS***********" << endl;
    unionColor = BLACK;//To return black-white picture from function
    weakObject = weakObject;
    garbageColor = BLUE;//To show objects discovered as not human
    MyVector<MyVector<Pt>> unionsTable2;
    assignCells(img, unionsTable2);

    cout << "UNIONS BEFORE HUMAN-PROPORTIONS CHECKING = " << unionsTable2.size() << endl;
    int humans2 = countHumans(unionsTable2);
    cout << "HUMANS QUANTITY DUE TO PROPORTIONS = " << humans2 << endl;

    return 0;
}
