//
//  File_Manager.hpp
//  DataBase
//
//  Created by 王诗媛 on 2017/10/20.
//  Copyright © 2017年 王诗媛. All rights reserved.
//

#ifndef File_Manager_hpp
#define File_Manager_hpp

#include "Records_Manager.h"
#include "fileio/FileManager.h"
#include "bufmanager/BufPageManager.h"

class File_Handler {
private:
    BufPageManager *mBufPageManager;
    FileManager *mFileManager;
public:
    File_Handler(FileManager *fm, BufPageManager *bpm);
    ~File_Handler();
    
    bool createFile(const char *fileName, int recordSize);
    bool destroyFile(const char *fileName);
    bool openFile(const char *fileName, Records_Manager *&records_M);
    bool closeFile(Records_Manager *records_M);
};


#endif /* File_Manager_hpp */
