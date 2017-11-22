//
//  Records_Manager.hpp
//  DataBase
//
//  Created by 王诗媛 on 2017/10/18.
//  Copyright © 2017年 王诗媛. All rights reserved.
//

#ifndef __RM_MANAGER_H__
#define __RM_MANAGER_H__

#include "File_Handle.h"

#include "fileio/FileManager.h"
#include "bufmanager/BufPageManager.h"

class RM_Manager {
private:
    BufPageManager *mBufPageManager;
    FileManager *mFileManager;
public:
    RM_Manager(FileManager *fm, BufPageManager *bpm);
    ~RM_Manager();
    
    bool createFile(const char *fileName, int recordSize);
    bool destroyFile(const char *fileName);
    bool openFile(const char *fileName, File_Handle *&fileHandle);
    bool closeFile(File_Handle *fileHandle);
};

#endif // __RM_MANAGER_H__
