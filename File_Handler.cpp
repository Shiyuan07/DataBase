//
//  File_Manager.cpp
//  DataBase
//
//  Created by 王诗媛 on 2017/10/20.
//  Copyright © 2017年 王诗媛. All rights reserved.
//

#include "File_Handler.h"

#include "utils/pagedef.h"

File_Handler::File_Handler(FileManager *fm, BufPageManager *bpm) {
    mFileManager = fm;
    mBufPageManager = bpm;
}

File_Handler::~File_Handler() {
    
}

bool File_Handler::createFile(const char *fileName, int recordSize) {
    if (recordSize > PAGE_SIZE) {
        return false;
    }
    bool flag = mFileManager->createFile(fileName);
    if (!flag) {
        return false;
    }
    int fileID;
    flag = mFileManager->openFile(fileName, fileID);
    if (!flag) {
        return false;
    }
    int index;
    BufType data = mBufPageManager->allocPage(fileID, 0, index, false);
    data[0] = (unsigned)recordSize;
    data[1] = (unsigned)(PAGE_SIZE / (recordSize + 1));
    data[2] = (unsigned)1;
    mBufPageManager->markDirty(index);
    mBufPageManager->writeBack(index);
    mFileManager->closeFile(fileID);
    return true;
}

bool File_Handler::destroyFile(const char *fileName) {
    return mFileManager->destroyFile(fileName);
}

bool File_Handler::openFile(const char *fileName, Records_Manager *&records_M) {
    int fileID;
    bool flag = mFileManager->openFile(fileName, fileID);
    if (!flag) {
        return false;
    }
    records_M = new Records_Manager(mBufPageManager, fileID);
    return true;
}

bool File_Handler::closeFile(Records_Manager *records_M) {
    int fileID = records_M->getFileID();
    mBufPageManager->close();
    delete records_M;
    return mFileManager->closeFile(fileID);
}
