//
//  SM_Manager.hpp
//  DataBaseE
//
//  Created by 王诗媛 on 2017/11/22.
//  Copyright © 2017年 王诗媛. All rights reserved.
//

#ifndef SM_Manager_h
#define SM_Manager_h
#include <string>
#include <vector>

#include "RM_Manager.h"
#include "IndexManager.h"
#include "base.h"

class SM_Manager {
private:
    IndexManager *mIXManager;
    RM_Manager *mRMManager;
    std::string mDBName;
    
public:
    SM_Manager(IndexManager *ix, RM_Manager *rm);
    ~SM_Manager();
    
    bool createDB(const char *DBName);
    bool dropDB(const char *DBName);
    bool useDB(const char *DBName);
    bool showDB(const char *DBName);
    
    bool createTable(const char *tableName, const char *primaryKey, const std::vector<AttrInfo> &attributes);
    bool dropTable(const char *tableName);
    bool showTable(const char *tableName);
    
    bool alterCheck(TableAttr tableAttr, const std::vector<Value> &values);
    bool alterForeign(TableAttr tableAttr, TableAttr foreignAttr);
    
    std::string getDBName();
};

#endif // __SM_MANAGER_H__#endif /* SM_Manager_h */
