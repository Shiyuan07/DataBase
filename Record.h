//
//  Record.h
//  DataBase
//
//  Created by 王诗媛 on 2017/10/18.
//  Copyright © 2017年 王诗媛. All rights reserved.
//

#ifndef Record_h
#define Record_h

#include <cstring>
#include <cstdio>

#include "utils/pagedef.h"

class RID {
public:
    int fileID;
    int pageID;
    int slotID;
    
public:
    RID() {
        fileID = pageID = slotID = -1;
    }
    RID(int fileID, int pageID, int slotID) {
        this->fileID = fileID;
        this->pageID = pageID;
        this->slotID = slotID;
    }
    ~RID() {}
    
    bool getAll(int &fileID, int &pageID, int &slotID) const {
        if (this->fileID == -1 || this->pageID == -1 || this->slotID == -1) {
            return false;
        }
        fileID = this->fileID;
        pageID = this->pageID;
        slotID = this->slotID;
        return true;
    }
    bool getPageNum(int &fileID, int &pageID) const {
        if (this->fileID == -1 || this->pageID == -1) {
            return false;
        }
        fileID = this->fileID;
        pageID = this->pageID;
        return true;
    }
    bool getSlotID(int &slotID) const {
        if (this->slotID == -1) {
            return false;
        }
        slotID = this->slotID;
        return true;
    }
    bool isValid() const {
        return this->fileID != -1 && this->pageID != -1 && this->slotID != -1;
    }
    
    bool operator < (const RID &rid) const {
        return fileID < rid.fileID || (fileID == rid.fileID && (pageID < rid.pageID || (pageID == rid.pageID && slotID < rid.slotID)));
    }
    
    bool operator == (const RID &rid) const {
        return fileID == rid.fileID && pageID == rid.pageID && slotID == rid.slotID;
    }
    
    bool operator != (const RID &rid) const {
        return !(*this == rid);
    }
};


class Record {
private:
    int mSize;
    char *mData;
    RID mRid;
    
public:
    Record() {
        mData = NULL;
    }
    Record(const char *pData, int size, RID rid) {
        mSize = size;
        mData = new char[mSize];
        memcpy(mData, pData, size);
        mRid = rid;
    }
    Record(const Record &rec) {
        // fprintf(stdout, "copy construction\n");
        //if (mData != NULL) {
        //	delete[] mData;
        //}
        mSize = rec.getSize();
        mData = new char[mSize];
        memcpy(mData, rec.getData(), mSize);
        mRid = rec.getRid();
    }
    ~Record() {
        // fprintf(stdout, "%p\n", mData);
        if (mData != NULL)
            delete[] mData;
        mData = NULL;
    }
    Record& operator =(const Record &rec) {
        // fprintf(stdout, "assignment operator\n");
        if (mData != NULL) {
            delete[] mData;
        }
        mSize = rec.getSize();
        mData = new char[mSize];
        memcpy(mData, rec.getData(), mSize);
        mRid = rec.getRid();
        return *this;
    }
    
    int getSize() const {
        return mSize;
    }
    char *getData() const {
        return mData;
    }
    RID getRid() const {
        return mRid;
    }
};

#endif /* Record_h */
