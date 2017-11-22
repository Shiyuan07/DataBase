//
//  File_Scan.hpp
//  DataBaseE
//
//  Created by 王诗媛 on 2017/11/22.
//  Copyright © 2017年 王诗媛. All rights reserved.
//

#ifndef File_Scan_h
#define File_Scan_h

#include <stdio.h>
#include <memory>

#include "File_Handle.h"
#include "Record.h"

#include "utils/base.h"

class FileScan {
private:
    int mCurIndex;
    std::vector<shared_ptr<Record> > mRecordVector;
    
public:
    FileScan();
    ~FileScan();
    
    bool openScan(File_Handle *fileHandle,
                  AttrType attrType,
                  int attrLength,
                  int attrOffset,
                  CompOp compOp,
                  void *value);
    bool getNextRec(Record &rec);
    bool closeScan();
    bool satisfyCondition(shared_ptr<Record> ptrRec,
                          AttrType attrType,
                          int attrLength,
                          int attrOffset,
                          CompOp compOp,
                          void *value);
    
};

#endif /* File_Scan_h */
