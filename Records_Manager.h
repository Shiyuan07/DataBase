//
//  Records_Manager.hpp
//  DataBase
//
//  Created by 王诗媛 on 2017/10/18.
//  Copyright © 2017年 王诗媛. All rights reserved.
//

#ifndef Records_Manager_hpp
#define Records_Manager_hpp

#include <vector>
#include <memory>

#include "utils/pagedef.h"
#include "bufmanager/BufPageManager.h"
#include "Record.h"
using namespace std;

class Records_Manager {
private:
    bool mModified;
    int mFileID;
    int mRecordSize;
    int mPageNum;
    int mSlotNum;
    int mRecordOffset;
    BufPageManager *mBufPageManager;
    std::vector<int> mAvailablePage;
    
private:
    RID getNewRid();
    void initPage(int pageID);
    void writeBackHeaderPage();
    void checkPageAvailable(int pageID);

    
public:
    Records_Manager(BufPageManager *bpm/*, int fileID*/);
    ~Records_Manager();
    
    //关于记录的操作
    bool getRec(const RID &rid, Record &rec) const;
    bool insertRec(const char *pData, RID &rid);
    bool deleteRec(const RID &rid);
    bool updateRec(const Record &rec);
};



#endif /* Records_Manager_hpp */
