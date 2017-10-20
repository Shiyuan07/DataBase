//
//  Records_Manager.cpp
//  DataBase
//
//  Created by 王诗媛 on 2017/10/18.
//  Copyright © 2017年 王诗媛. All rights reserved.
//

#include <vector>
#include <memory>
#include <cstdio>
#include <cassert>

#include "Records_Manager.h"

using namespace std;

Records_Manager::Records_Manager(BufPageManager *bpm, int fileID) {
    mBufPageManager = bpm;
    mFileID = fileID;
    mModified = false;
    int index;
    BufType uData = mBufPageManager->allocPage(mFileID, 0, index, true);
    mRecordSize = (int)uData[0];
    mSlotNum = (int)uData[1];
    mRecordOffset = mSlotNum;
    mPageNum = (int)uData[2];
    char *data = (char *)uData;
    data = data + 12;
    mAvailablePage.push_back(false);
    for (int i = 1; i < mPageNum; ++i) {
        if (data[i] == 0)
            mAvailablePage.push_back(true);
        else
            mAvailablePage.push_back(false);
    }
    mBufPageManager->release(index);
}

Records_Manager::~Records_Manager() {
    if (mModified) {
        writeBackHeaderPage();
    }
    mBufPageManager->close();
}

RID Records_Manager::getNewRid() {
    int pageID = -1, slotID = -1;
    //fprintf(stdout, "PageNum = %d\n", mPageNum);
    for (int i = 1; i < mPageNum; ++i) {
        //fprintf(stdout, "size = %d\n", mAvailablePage.size());
        //fprintf(stdout, "i = %d, mAvailable = %d\n", i, mAvailablePage[i]);
        if (mAvailablePage[i]) {
            pageID = i;
            break;
        }
    }
    if (pageID == -1) {
        pageID = mPageNum;
        slotID = 0;
        ++mPageNum;
        mAvailablePage.push_back(true);
        mModified = true;
        initPage(pageID);
    } else {
        int index;
        BufType uData = mBufPageManager->getPage(mFileID, pageID, index);
        char *data = (char *)uData;
        for (int i = 0; i < mSlotNum; ++i) {
            if (data[i] == 0) {
                slotID = i;
                break;
            }
        }
    }
    //fprintf(stdout, "new rid = (%d,%d,%d)\n", mFileID, pageID, slotID);
    return RID(mFileID, pageID, slotID);
}

void Records_Manager::initPage(int pageID) {
    int index;
    BufType uData = mBufPageManager->getPage(mFileID, pageID, index);
    char *data = (char *)uData;
    memset(data, 0, mSlotNum);
    mBufPageManager->markDirty(index);
}

void Records_Manager::writeBackHeaderPage() {
    int index;
    BufType uData = mBufPageManager->allocPage(mFileID, 0, index, false);
    uData[0] = mRecordSize;
    uData[1] = mSlotNum;
    uData[2] = mPageNum;
    char *data = (char *)uData;
    data = data + 12;
    for (int i = 1; i < mPageNum; ++i) {
        if (mAvailablePage[i])
            data[i] = 0;
        else
            data[i] = 1;
    }
    mBufPageManager->markDirty(index);
    mBufPageManager->writeBack(index);
}

void Records_Manager::checkPageAvailable(int pageID) {
    int index;
    BufType uData = mBufPageManager->getPage(mFileID, pageID, index);
    char *data = (char *)uData;
    bool available = false;
    for (int i = 0; i < mSlotNum; ++i)
        if (data[i] == 0) {
            available = true;
            break;
        }
    //cout << "pageID = " << pageID << "available = " << available << endl;
    if (mAvailablePage[pageID] != available) {
        mModified = true;
        mAvailablePage[pageID] = available;
    }
}

int Records_Manager::getFileID() const {
    return mFileID;
}

int Records_Manager::getPageNum() const {
    return mPageNum;
}

bool Records_Manager::getAllRecFromPage(int pageID, vector<shared_ptr<Record> > &recordVector) {
    assert(pageID > 0);
    int index;
    BufType uData = mBufPageManager->getPage(mFileID, pageID, index);
    char *data = (char *)uData;
    recordVector.clear();
    for (int i = 0; i < mSlotNum; ++i) {
        if (data[i] == 1) {
            //fprintf(stdout, "i = %d p = %x\n", i, uData);
            char *curData = data + mRecordOffset + mRecordSize * i;
            shared_ptr<Record> ptrRec(new Record(curData, mRecordSize, RID(mFileID, pageID, i)));
            recordVector.push_back(ptrRec);
        }
    }
    return true;
}

bool Records_Manager::getRec(const RID &rid, Record &rec) const {
    int fileID, pageID, slotID;
    if (!rid.getAll(fileID, pageID, slotID)) {
        return false;
    }
    int index;
    BufType uData = mBufPageManager->getPage(fileID, pageID, index);
    char *data = (char *)uData;
    if (data[slotID] == 0) {
        return false;
    }
    data = data + mRecordOffset + mRecordSize * slotID;
    rec = Record(data, mRecordSize, rid);
    return true;
}

bool Records_Manager::insertRec(const char *pData, RID &rid) {
    rid = getNewRid();
    int fileID, pageID, slotID;
    if (!rid.getAll(fileID, pageID, slotID)) {
        //fprintf(stdout, "insert record pageID = %d, slotID = %d\n", pageID, slotID);
        return false;
    }
    int index;
    BufType uData = mBufPageManager->getPage(fileID, pageID, index);
    char *data = (char *)uData;
    data[slotID] = 1;
    checkPageAvailable(pageID);
    data = data + mRecordOffset + mRecordSize * slotID;
    memcpy(data, pData, mRecordSize);
    mBufPageManager->markDirty(index);
    return true;
}

bool Records_Manager::deleteRec(const RID &rid) {
    int fileID, pageID, slotID;
    if (!rid.getAll(fileID, pageID, slotID)) {
        return false;
    }
    int index;
    BufType uData = mBufPageManager->getPage(fileID, pageID, index);
    char *data = (char *)uData;
    data[slotID] = 0;
    checkPageAvailable(pageID);
    mBufPageManager->markDirty(index);
    return true;
}

bool Records_Manager::updateRec(const Record &rec) {
    RID rid = rec.getRid();
    int fileID, pageID, slotID;
    if (!rid.getAll(fileID, pageID, slotID)) {
        return false;
    }
    int index;
    BufType uData = mBufPageManager->getPage(fileID, pageID, index);
    char *data = (char *)uData;
    data = data + mRecordOffset + mRecordSize * slotID;
    memcpy(data, rec.getData(), mRecordSize);
    mBufPageManager->markDirty(index);
    return true;
}

bool Records_Manager::forcePage(int pageID) const {
    int index = -1;
    mBufPageManager->getPage(mFileID, pageID, index);
    if (index == -1) {
        return false;
    }
    mBufPageManager->writeBack(index);
    return true;
}
