//
//  SM_Manager.cpp
//  DataBaseE
//
//  Created by 王诗媛 on 2017/11/22.
//  Copyright © 2017年 王诗媛. All rights reserved.
//

#include <vector>
#include <cassert>
#include "SM_Manager.h"

using namespace std;

extern const int MAX_NAME_LEN;
extern const int MAX_ATTR_LEN;

SM_Manager::SM_Manager(IndexManager *ix, RM_Manager *rm) {
    mIXManager = ix;
    mRMManager = rm;
    mDBName = "";
}

SM_Manager::~SM_Manager() {
    
}

bool SM_Manager::createDB(const char *DBName) {
    int len = strlen(DBName);
    assert(len < MAX_NAME_LEN);
    
    char command[MAX_NAME_LEN + 20];
    sprintf(command, "mkdir %s", DBName);
    system(command);
    
    char relcat[MAX_NAME_LEN + 20];
    sprintf(relcat, "%s/relcat", DBName);
    mRMManager->createFile(relcat, MAX_NAME_LEN + 12);
    
    char attrcat[MAX_NAME_LEN + 20];
    sprintf(attrcat, "%s/attrcat", DBName);
    mRMManager->createFile(attrcat, MAX_NAME_LEN * 2 + 20 + 4 + MAX_NAME_LEN * 2 + 4 + MAX_ATTR_LEN);
    
    fprintf(stderr, "create database %s finished\n", DBName);
    return true;
}

bool SM_Manager::dropDB(const char *DBName) {
    int len = strlen(DBName);
    char command[MAX_NAME_LEN + 20];
    sprintf(command, "rm -r %s", DBName);
    system(command);
    fprintf(stderr, "drop database %s finished\n", DBName);
    return true;
}

bool SM_Manager::useDB(const char *DBName) {
    FILE *DBFile = fopen("DBUSE.tmp", "w");
    fprintf(DBFile, "%s", DBName);
    fclose(DBFile);
    mDBName = string(DBName);
    return true;
}

bool SM_Manager::showDB(const char *DBName) {
    File_Handle *handle;
    char relcat[MAX_NAME_LEN + 20];
    sprintf(relcat, "%s/relcat", DBName);
    mRMManager->openFile(relcat, handle);
    
    int pageNum = handle->getPageNum();
    vector<shared_ptr<Record> > records;
    fprintf(stdout, "DATABASE %s:\n", DBName);
    for (int i = 1; i < pageNum; ++i) {
        handle->getAllRecFromPage(i, records);
        int num = records.size();
        for (int j = 0; j < num; ++j) {
            shared_ptr<Record> rec = records[j];
            char *pData = rec->getData();
            fprintf(stdout, "  TABLE %s\n", pData);
        }
    }
    mRMManager->closeFile(handle);
    return true;
}

bool SM_Manager::createTable(const char *tableName, const char *primaryKey, const vector<AttrInfo> &attributes) {
    //cerr << "SM_Manager::createTable begin" << endl;
    getDBName();
    assert(mDBName != "");
    
    int attrCount = attributes.size();
    int recordSize = 0;
    int primaryNum = -1;
    for (int i = 0; i < attrCount; ++i) {
        recordSize += attributes[i].attrLength;
        if (strcmp(primaryKey, attributes[i].attrName) == 0) {
            primaryNum = i;
        }
    }
    if (primaryNum != -1) {
        string fullTableName = mDBName + "/" + string(tableName);
        mIXManager->CreateIndex(fullTableName.c_str(), primaryNum, attributes[primaryNum].attrType, attributes[primaryNum].attrLength);
    }
    
    string fullTableName = mDBName + "/" + string(tableName);
    mRMManager->createFile(fullTableName.c_str(), recordSize);
    
    File_Handle *handle;
    string relcat = mDBName + "/relcat";
    mRMManager->openFile(relcat.c_str(), handle);
    RID rid;
    char *pData = new char[MAX_NAME_LEN + 12];
    int len = strlen(tableName);
    assert(len < MAX_NAME_LEN);
    for (int i = 0; i < MAX_NAME_LEN; ++i) {
        if (i < len)
            pData[i] = tableName[i];
        else
            pData[i] = 0;
    }
    BufType data = (BufType)(pData + MAX_NAME_LEN);
    data[0] = recordSize;
    data[1] = attrCount;
    data[2] = primaryNum;
    handle->insertRec(pData, rid);
    mRMManager->closeFile(handle);
    delete[] pData;
    
    string attrcat = mDBName + "/attrcat";
    mRMManager->openFile(attrcat.c_str(), handle);
    int offset = 0;
    for (int i = 0; i < attrCount; ++i) {
        pData = new char[MAX_NAME_LEN * 2 + 20 + 4 + MAX_NAME_LEN * 2 + 4 + MAX_ATTR_LEN];
        for (int i = 0; i < MAX_NAME_LEN; ++i) {
            if (i < len)
                pData[i] = tableName[i];
            else
                pData[i] = 0;
        }
        int attrLen = strlen(attributes[i].attrName);
        assert(attrLen < MAX_NAME_LEN);
        for (int j = 0; j < MAX_NAME_LEN; ++j) {
            if (j < attrLen)
                pData[MAX_NAME_LEN + j] = attributes[i].attrName[j];
            else
                pData[MAX_NAME_LEN + j] = 0;
        }
        BufType data = (BufType)(pData + MAX_NAME_LEN * 2);
        data[0] = offset;
        data[1] = (unsigned int)attributes[i].attrType;
        data[2] = attributes[i].attrLength;
        if (primaryNum == i)
            data[3] = 0;
        else
            data[3] = -1;
        data[4] = (unsigned int)attributes[i].nullable;
        data[5] = 0;
        
        data = (BufType)(pData + MAX_NAME_LEN * 2 + 20 + 4 + MAX_NAME_LEN * 2);
        data[0] = 0;
        
        handle->insertRec(pData, rid);
        offset += attributes[i].attrLength;
        delete[] pData;
    }
    mRMManager->closeFile(handle);
    //cerr << "SM_Manager::createTable end" << endl;
    fprintf(stderr, "create table %s finished\n", tableName);
    return true;
}

bool SM_Manager::dropTable(const char *tableName) {
    getDBName();
    File_Handle *handle;
    string relcat = mDBName + "/relcat";
    mRMManager->openFile(relcat.c_str(), handle);
    
    int indexNo = -1;
    string tableNameStr(tableName);
    int pageNum = handle->getPageNum();
    vector<shared_ptr<Record> > records;
    for (int i = 1; i < pageNum; ++i) {
        handle->getAllRecFromPage(i, records);
        int num = records.size();
        for (int j = 0; j < num; ++j) {
            shared_ptr<Record> rec = records[j];
            char *pData = rec->getData();
            string tableNameTmp(pData);
            if (tableNameStr != tableNameTmp)
                continue;
            BufType data = (BufType)(pData + MAX_NAME_LEN);
            indexNo = data[2];
        }
    }
    mRMManager->closeFile(handle);
    string fullTableName = mDBName + "/" + string(tableName);
    if (indexNo != -1)
        mIXManager->DestroyIndex(fullTableName.c_str(), indexNo);
    bool resFlag = mRMManager->destroyFile(fullTableName.c_str());
    fprintf(stderr, "drop table %s finished\n", tableName);
    return resFlag;
}

bool SM_Manager::showTable(const char *tableName) {
    getDBName();
    File_Handle *handle;
    string attrcat = mDBName + "/attrcat";
    mRMManager->openFile(attrcat.c_str(), handle);
    
    string tableNameStr(tableName);
    int pageNum = handle->getPageNum();
    vector<shared_ptr<Record> > records;
    fprintf(stdout, "TABLE %s:\n", tableName);
    for (int i = 1; i < pageNum; ++i) {
        handle->getAllRecFromPage(i, records);
        int num = records.size();
        for (int j = 0; j < num; ++j) {
            shared_ptr<Record> rec = records[j];
            char *pData = rec->getData();
            string tableNameTmp(pData);
            if (tableNameStr != tableNameTmp)
                continue;
            fprintf(stdout, "  attrName: %s, ", pData + MAX_NAME_LEN);
            BufType data = (BufType)(pData + MAX_NAME_LEN * 2);
            fprintf(stdout, "attrType: ");
            switch (data[1]) {
                case INTEGER: fprintf(stdout, "int, "); break;
                case FLOAT: fprintf(stdout, "float, "); break;
                case STRING: fprintf(stdout, "string, "); break;
                case CHAR: fprintf(stdout, "char, "); break;
                default: break;
            }
            fprintf(stdout, "attrLength: %d, ", data[2]);
            fprintf(stdout, "indexNo: %d, ", data[3]);
            fprintf(stdout, "nullable: %d\n", data[4]);
        }
    }
    mRMManager->closeFile(handle);
    return true;
}

string SM_Manager::getDBName() {
    if (mDBName == "") {
        FILE *DBFile = fopen("DBUSE.tmp", "r");
        char DBName[20];
        fscanf(DBFile, "%s", DBName);
        fclose(DBFile);
        mDBName = string(DBName);
    }
    return mDBName;
}

bool SM_Manager::alterCheck(TableAttr tableAttr, const std::vector<Value> &values) {
    //fprintf(stdout, "enter SM_Manager::alterCheck\n");
    getDBName();
    File_Handle *handle;
    string attrcat = mDBName + "/attrcat";
    mRMManager->openFile(attrcat.c_str(), handle);
    
    string tableNameStr(tableAttr.tableName);
    string attrNameStr(tableAttr.attrName);
    int pageNum = handle->getPageNum();
    vector<shared_ptr<Record> > records;
    for (int i = 1; i < pageNum; ++i) {
        handle->getAllRecFromPage(i, records);
        int num = records.size();
        for (int j = 0; j < num; ++j) {
            shared_ptr<Record> rec = records[j];
            char *pData = rec->getData();
            string tableNameTmp(pData);
            if (tableNameStr != tableNameTmp)
                continue;
            string attrNameTmp(pData + MAX_NAME_LEN);
            if (attrNameStr != attrNameTmp)
                continue;
            char *nData = new char[MAX_NAME_LEN * 2 + 20 + 4 + MAX_NAME_LEN * 2 + 4 + MAX_ATTR_LEN];
            memcpy(nData, pData, MAX_NAME_LEN * 2 + 20 + 4 + MAX_NAME_LEN * 2 + 4 + MAX_ATTR_LEN);
            handle->deleteRec(rec->getRid());
            BufType data = (BufType)(nData + MAX_NAME_LEN * 2 + 20 + 4 + MAX_NAME_LEN * 2);
            //fprintf(stdout, "values.size() = %d\n", (int)values.size());
            data[0] = values.size();
            char *cData = nData + MAX_NAME_LEN * 2 + 20 + 4 + MAX_NAME_LEN * 2 + 4;
            for (int k = 0; k < values.size(); ++k) {
                int len = min(MAX_ATTR_LEN / values.size(), strlen((char *)values[k].data));
                cData += MAX_ATTR_LEN / values.size() * k;
                memcpy(cData, values[k].data, len);
                cData[len] = 0;
            }
            RID rid;
            handle->insertRec(nData, rid);
        }
    }
    mRMManager->closeFile(handle);
    //fprintf(stdout, "leave SM_Manager::alterCheck\n");
    return true;
}

bool SM_Manager::alterForeign(TableAttr tableAttr, TableAttr foreignAttr) {
    //fprintf(stdout, "enter SM_Manager::alterForeign\n");
    //fprintf(stdout, "%s\n%s\n", tableAttr.tableName, tableAttr.attrName);
    //fprintf(stdout, "%s\n%s\n", foreignAttr.tableName, foreignAttr.attrName);
    getDBName();
    File_Handle *handle;
    string attrcat = mDBName + "/attrcat";
    mRMManager->openFile(attrcat.c_str(), handle);
    
    string tableNameStr(tableAttr.tableName);
    string attrNameStr(tableAttr.attrName);
    int pageNum = handle->getPageNum();
    vector<shared_ptr<Record> > records;
    for (int i = 1; i < pageNum; ++i) {
        handle->getAllRecFromPage(i, records);
        int num = records.size();
        for (int j = 0; j < num; ++j) {
            shared_ptr<Record> rec = records[j];
            char *pData = rec->getData();
            string tableNameTmp(pData);
            if (tableNameStr != tableNameTmp)
                continue;
            string attrNameTmp(pData + MAX_NAME_LEN);
            if (attrNameStr != attrNameTmp)
                continue;
            //fprintf(stdout, "%s %s\n", tableNameTmp.c_str(), attrNameTmp.c_str());
            char *nData = new char[MAX_NAME_LEN * 2 + 20 + 4 + MAX_NAME_LEN * 2 + 4 + MAX_ATTR_LEN];
            memcpy(nData, pData, MAX_NAME_LEN * 2 + 20 + 4 + MAX_NAME_LEN * 2 + 4 + MAX_ATTR_LEN);
            handle->deleteRec(rec->getRid());
            BufType data = (BufType)(nData + MAX_NAME_LEN * 2 + 20);
            data[0] = 1;
            char *cData = nData + MAX_NAME_LEN * 2 + 20 + 4;
            memcpy(cData, foreignAttr.tableName, MAX_NAME_LEN);
            memcpy(cData + MAX_NAME_LEN, foreignAttr.attrName, MAX_NAME_LEN);
            RID rid;
            handle->insertRec(nData, rid);
        }
    }
    mRMManager->closeFile(handle);
    //fprintf(stdout, "leave SM_Manager::alterForeign\n");
    return true;
}

