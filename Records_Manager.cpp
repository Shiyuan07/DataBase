//
//  Records_Manager.cpp
//  DataBase
//
//  Created by 王诗媛 on 2017/10/18.
//  Copyright © 2017年 王诗媛. All rights reserved.
//

#include "Records_Manager.h"

Records_Manager::Records_Manager(BufPageManager *bpm/*, int fileID*/){
    this->mBufPageManager = bpm;
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
