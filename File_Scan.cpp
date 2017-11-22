//
//  File_Scan.cpp
//  DataBaseE
//
//  Created by 王诗媛 on 2017/11/22.
//  Copyright © 2017年 王诗媛. All rights reserved.
//

#include "File_Scan.h"
#include <cmath>
#include <memory>

#include "Record.h"

using namespace std;

extern const double EPS;

FileScan::FileScan() {}

FileScan::~FileScan() {}

bool FileScan::satisfyCondition(shared_ptr<Record> ptrRec,
                                   AttrType attrType,
                                   int attrLength,
                                   int attrOffset,
                                   CompOp compOp,
                                   void *value)
{
    if (compOp == NO_OP || value == NULL) {
        return true;
    }
    char *data = ptrRec->getData();
    data = data + attrOffset;
    if (attrType == INTEGER) {
        int attrValue = *((int *)data);
        int intValue = *((int *)value);
        switch (compOp) {
            case EQ_OP: return attrValue == intValue;
            case LT_OP: return attrValue < intValue;
            case GT_OP: return attrValue > intValue;
            case LE_OP: return attrValue <= intValue;
            case GE_OP: return attrValue >= intValue;
            case NE_OP: return attrValue != intValue;
            default: return false;
        }
    } else if (attrType == FLOAT) {
        float attrValue = *((float *)data);
        float floatValue = *((float *)value);
        switch (compOp) {
            case EQ_OP: return fabs(attrValue - floatValue) < EPS;
            case LT_OP: return attrValue < floatValue;
            case GT_OP: return attrValue > floatValue;
            case LE_OP: return attrValue <= floatValue;
            case GE_OP: return attrValue >= floatValue;
            case NE_OP: return fabs(attrValue - floatValue) > EPS;
            default: return false;
        }
    } else if (attrType == STRING) {
        char *stringValue = (char *)value;
        int cmpResult = strncmp(data, stringValue, attrLength);
        // for (int i = 0; i < attrLength; ++i) {
        //     if (data[i] < stringValue[i]) {
        //         cmpResult = -1;
        //         break;
        //     } else if (data[i] > stringValue[i]) {
        //         cmpResult = 1;
        //         break;
        //     }
        // }
        switch (compOp) {
            case EQ_OP: return cmpResult == 0;
            case LT_OP: return cmpResult == -1;
            case GT_OP: return cmpResult == 1;
            case LE_OP: return cmpResult <= 0;
            case GE_OP: return cmpResult >= 0;
            case NE_OP: return cmpResult != 0;
            default: return false;
        }
    }
    return false;
}

bool FileScan::openScan(File_Handle *fileHandle,
                           AttrType attrType,
                           int attrLength,
                           int attrOffset,
                           CompOp compOp,
                           void *value)
{
    mCurIndex = 0;
    mRecordVector.clear();
    vector<shared_ptr<Record> > tmpRecordVector;
    int pageNum = fileHandle->getPageNum();
    for (int i = 1; i < pageNum; ++i) {
        bool flag = fileHandle->getAllRecFromPage(i, tmpRecordVector);
        if (!flag) {
            return false;
        }
        vector<shared_ptr<Record> >::iterator iter = tmpRecordVector.begin();
        for (; iter != tmpRecordVector.end(); ++iter) {
            if (satisfyCondition(*iter, attrType, attrLength, attrOffset, compOp, value)) {
                mRecordVector.push_back(*iter);
            }
        }
    }
    return true;
}

bool FileScan::getNextRec(Record &rec) {
    if (mCurIndex < mRecordVector.size()) {
        rec = *mRecordVector[mCurIndex];
        ++mCurIndex;
        return true;
    }
    return false;
}

bool FileScan::closeScan() {
    mCurIndex = 0;
    mRecordVector.clear();
    return true;
}
