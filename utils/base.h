//
//  Base.h
//  DataBaseE
//
//  Created by 王诗媛 on 2017/11/22.
//  Copyright © 2017年 王诗媛. All rights reserved.
//

#ifndef Base_h
#define Base_h
#include <string>
#include <vector>

const int MAX_NAME_LEN = 20;
const int MAX_ATTR_LEN = 24;
const double EPS = 1e-6;

enum AttrType {
    INTEGER = 0,
    FLOAT,
    STRING,
    LONGLONG,
    CHAR,
    NOTYPE
};

enum CompOp {
    EQ_OP,
    LT_OP,
    GT_OP,
    LE_OP,
    GE_OP,
    NE_OP,
    NO_OP,
    LK_OP,
    NULL_OP
};

enum GroupFunc {
    AVG_FUNC = 0,
    SUM_FUNC,
    MIN_FUNC,
    MAX_FUNC,
    NO_FUNC
};

struct AttrInfo {
    bool nullable;
    const char *attrName;
    AttrType attrType;
    int attrLength;
};

struct AttrInfoEx {
    bool nullable;
    std::string attrName;
    int offset;
    AttrType attrType;
    int attrLength;
    int indexNo;
    bool isForeignKey;
    std::string foreignTable;
    std::string foreignAttr;
    bool isCheck;
    std::vector<std::string> values;
};

struct TableAttr {
    const char *tableName;
    const char *attrName;
};

struct TableAttrEx {
    const char *tableName;
    const char *attrName;
    GroupFunc func;
};

struct Value {
    AttrType attrType;
    void *data;
};

struct Condition {
    TableAttr lAttr;
    CompOp op;
    bool rIsValue;
    TableAttr rAttr;
    Value rValue;
};

#endif /* Base_h */
