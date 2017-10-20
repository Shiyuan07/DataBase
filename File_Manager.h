//
//  File_Manager.hpp
//  DataBase
//
//  Created by 王诗媛 on 2017/10/20.
//  Copyright © 2017年 王诗媛. All rights reserved.
//

#ifndef File_Manager_hpp
#define File_Manager_hpp

#include "fileio/FileManager.h"
#include "bufmanager/BufPageManager.h"

class File_Manager {
private:
    BufPageManager *mBufPageManager;
    FileManager *mFileManager;
public:
    RM_Manager(FileManager *fm, BufPageManager *bpm);
    ~RM_Manager();
    
    bool createFile(const char *fileName, int recordSize);
    bool destroyFile(const char *fileName);
    bool openFile(const char *fileName, RM_FileHandle *&fileHandle);
    bool closeFile(RM_FileHandle *fileHandle);
};


#endif /* File_Manager_hpp */
