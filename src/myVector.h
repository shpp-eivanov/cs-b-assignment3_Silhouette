/*
 * File: myVector.h
 * -------------
 * This file implements Vector structure with my implelmentaion
 */

#ifndef _myVector_h
#define _myVector_h

#include "error.h"
#include <iostream>
#include <stdlib.h>

/*
 * Class: MyVector<ValueType>
 * -----------------------
 */
template <typename ValueType>
class MyVector {
public:
    /*
     * Constructor: MyVector
     * Usage: MyVector<ValueType> stack;
     * ------------------------------
     * Initializes a new empty vector.
     */
    MyVector();

    /*
     * Destructor: ~MyVector
     * ------------------
     * Frees any heap storage associated with this vector.
     */
    virtual ~MyVector();
    
    /*
     * Method: add
     * Usage: myVec.add(value);
     * -------------------------
     * Adds the specified value onto the end of this vector.
     */
    void add(const ValueType& value);
    
    /*
     * Method: isEmpty
     * Usage: if (myVec.isEmpty()) ...
     * -------------------------------
     * Returns <code>true</code> if this vector contains no elements.
     */
    bool isEmpty() const;
    
    /*
     * Method: get
     * Usage: ValueType val = myVec.get(index);
     * --------------------------------------
     * Returns the element at the specified index in this vector.  This
     * method signals an error if the index is not in the array range.
     */
    const ValueType& get(int index) const;

    /*
     * Method: remove
     * Usage: myVec.remove(index);
     * -------------------------
     * Removes the element at the specified index from this vector.
     * All subsequent elements are shifted one position to the left.  This
     * method signals an error if the index is outside the array range.
     */
    void remove(int index);

    void removeValue(ValueType& value);

    /*
     * Method: size
     * Usage: int nElems = myVec.size();
     * -------------------------------
     * Returns the number of elements in this vector.
     */
    int size() const;

    /* Checks if this vector contain such value */
    bool isContains(const ValueType& value)const;

    /*
     * Deep copying support
     * --------------------
     * This copy constructor and operator= are defined to make a deep copy,
     * making it possible to pass or return vectors by value and assign
     * from one vector to another.
     */
    MyVector(const MyVector& src);
    MyVector& operator =(const MyVector& src);

private:
    /*
     * Implementation notes: Vector data structure
     * -------------------------------------------
     * The elements of the Vector are stored in a dynamic array of
     * the specified element type.  If the space in the array is ever
     * exhausted, the implementation doubles the array capacity.
     */

    /* Instance variables */
    ValueType* elements;        /* A dynamic array of the elements   */
    int capacity;               /* The allocated size of the array   */
    int count;                  /* The number of elements in use     */

    void expandCapacity();
    void deepCopy(const MyVector& src);
};

/* Implementation section */

/*
 * Implementation notes: Vector constructor and destructor
 * -------------------------------------------------------
 * The constructor allocates storage for the dynamic array
 * and initializes the other fields of the object.  The
 * destructor frees the memory used for the array.
 */
template <typename ValueType>
MyVector<ValueType>::MyVector() {
    capacity = 2;
    count = 0;
    elements = new ValueType[capacity];
}

/*
 * Implementation notes: copy constructor and assignment operator
 * --------------------------------------------------------------
 * The constructor and assignment operators follow a standard paradigm,
 * as described in the associated textbook.
 */
template <typename ValueType>
MyVector<ValueType>::MyVector(const MyVector& src) {
    deepCopy(src);
}

template <typename ValueType>
MyVector<ValueType>::~MyVector() {
    if (elements != NULL) {
        //std::cout << "Bye-bye vec!" << std::endl;
        delete[] elements;
    }
}

/*
 * Implementation notes: insert, remove, add
 * -----------------------------------------
 * These methods must shift the existing elements in the array to
 * make room for a new element or to close up the space left by a
 * deleted one.
 */
template <typename ValueType>
void MyVector<ValueType>::add(const ValueType& value) {
    if(count == capacity)expandCapacity();
    elements[count] = value;
    count++;
}

/*
 * Implementation notes: expandCapacity
 * ------------------------------------
 * This function doubles the array capacity, copies the old elements
 * into the new array, and then frees the old one.
 */
template <typename ValueType>
void MyVector<ValueType>::expandCapacity() {
    ValueType *oldArray = elements;
    capacity = 2*capacity;
    elements = new ValueType[capacity];
    for(int i = 0; i < count; i++){
        elements[i] = oldArray[i];
    }
    delete []oldArray;
}

template <typename ValueType>
const ValueType &MyVector<ValueType>::get(int index) const {
    if((count == 0) || (index >= count))error("get: Attempting to get is failed");
    return elements[index];
}

template <typename ValueType>
bool MyVector<ValueType>::isEmpty() const {
    return count == 0;
}

template <typename ValueType>
int MyVector<ValueType>::size() const {
    return count;
}

template <typename ValueType>
void MyVector<ValueType>::remove(int index) {
    if((index < 0) || (index >= count)){error("remove: Attempting to remove is failed");
    }else{
        for (int i = index; i < count - 1; i++) {
            elements[i] = elements[i + 1];
        }
        count--;
    }
}

template <typename ValueType>
void MyVector<ValueType>::removeValue(ValueType& value) {
    if(!isContains(value)){error("remove: Attempting to remove not existing element");
    }else{
        for (int i = 0; i < count; i++) {
            if(elements[i] == value){
                remove(i);
            }
        }
    }
}

template <typename ValueType>
MyVector<ValueType> & MyVector<ValueType>::operator =(const MyVector& src) {
    if (this != &src) {
        if (elements != NULL) {
            delete[] elements;
        }
        deepCopy(src);
    }
    return *this;
}

template <typename ValueType>
void MyVector<ValueType>::deepCopy(const MyVector& src) {
    count = capacity = src.count;
    elements = (capacity == 0) ? NULL : new ValueType[capacity];
    for (int i = 0; i < count; i++) {
        elements[i] = src.elements[i];
    }
}

/* Checks if this vector contain such value */
template <typename ValueType>
bool MyVector<ValueType>::isContains(const ValueType& value)const{
    bool contains = false;
    for(int i = 0; i < count; i++){
        if(elements[i] == value){
            contains = true;
            break;
        }
    }
    return contains;
}



#endif
