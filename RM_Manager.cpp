#include "RM_Manager.h"

#include "utils/pagedef.h"

RM_Manager::RM_Manager(FileManager *fm, BufPageManager *bpm) {
    mFileManager = fm;
    mBufPageManager = bpm;
}

RM_Manager::~RM_Manager() {
    
}

bool RM_Manager::createFile(const char *fileName, int recordSize) {
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

bool RM_Manager::destroyFile(const char *fileName) {
    return mFileManager->destroyFile(fileName);
}

bool RM_Manager::openFile(const char *fileName, File_Handle *&fileHandle) {
    int fileID;
    bool flag = mFileManager->openFile(fileName, fileID);
    if (!flag) {
        return false;
    }
    fileHandle = new File_Handle(mBufPageManager, fileID);
    return true;
}

bool RM_Manager::closeFile(File_Handle *fileHandle) {
    int fileID = fileHandle->getFileID();
    mBufPageManager->close();
    delete fileHandle;
    return mFileManager->closeFile(fileID);
}
