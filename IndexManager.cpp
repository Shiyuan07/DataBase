#include <iostream>
#include "IndexManager.h"

using namespace std;

IndexManager::IndexManager (FileManager *fm, BufPageManager* bpm)
{
  mFileManager = fm;
  mBufPageManager = bpm;
}

IndexManager::~IndexManager() {

}

char* IndexManager::makeFileName(const char *fileName, int indexNo) {
    char*name;
    /*
  	memset(name, 0, sizeof(name));
  	sprintf(name, "%s.%d.index", fileName, indexNo);
    */
    return name;
}

bool IndexManager::CreateIndex(const char *fileName, int indexNo, AttrType attrType, int attrLength) {
    char fileNameindex[100];
    memset(fileNameindex, 0, sizeof(fileNameindex));
    sprintf(fileNameindex, "%s.%d.index", fileName, indexNo);
    if (access(fileNameindex, 0) != -1) {
  		return false;
  	}
    int fileID, index;
    mFileManager->createFile(fileNameindex);
    mFileManager->openFile(fileNameindex, fileID);
    char* firstPage = (char*)mBufPageManager->allocPage(fileID, 0, index, false);
    IndexTreeHeader treeheader;
    treeheader.pageNum = 1;
  	treeheader.firstFreePage = 1;
  	treeheader.rootPage = -1;
  	treeheader.attrType = attrType;
  	treeheader.attrLength = attrLength;
  	treeheader.maxN = (PAGE_SIZE - 192) / (sizeof(RID) + attrLength) - 2;
  	memcpy(firstPage, (char*)(&treeheader), sizeof(IndexTreeHeader));

    mBufPageManager->markDirty(index);
    mBufPageManager->writeBack(index);
    mFileManager->closeFile(fileID);
    return true;
}

bool IndexManager::DestroyIndex(const char *fileName, int indexNo) {
    char fileNameindex[100];
    memset(fileNameindex, 0, sizeof(fileNameindex));
    sprintf(fileNameindex, "%s.%d.index", fileName, indexNo);
    return mFileManager->destroyFile(fileNameindex);
}

bool IndexManager::OpenIndex(const char *fileName, int indexNo, IndexHandle &indexHandle) {
    char fileNameindex[100];
    memset(fileNameindex, 0, sizeof(fileNameindex));
    sprintf(fileNameindex, "%s.%d.index", fileName, indexNo);
    int fileID;
    bool flag = mFileManager->openFile(fileNameindex, fileID);
    if (!flag) {
        return false;
    }
    indexHandle = IndexHandle(mBufPageManager, fileID);
    return true;
}

bool IndexManager::CloseIndex(IndexHandle &indexHandle) {
    int fileID = indexHandle.getFileID();
    indexHandle.ForcePages();
    mFileManager->closeFile(fileID);
    mBufPageManager -> close();
    return true;
}
