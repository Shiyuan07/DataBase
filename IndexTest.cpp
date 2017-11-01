#include <iostream>

#include "IndexManager.h"
#include "IndexHandle.h"

using namespace std;

int main() {
    FileManager *fm = new FileManager();
    BufPageManager *bpm = new BufPageManager(fm);
    IndexManager *im = new IndexManager(fm, bpm);
    bool flag;
    flag = im->CreateIndex("test", 0, INTEGER, 4);
    if (flag) {
        cout << "create index successfully" << endl;
    }
    IndexHandle indexHandle;
    flag = im->OpenIndex("test", 0, indexHandle);
    if (flag) {
        cout << "open index successfully" << endl;
    }


    int num = 10;
    for (int i = 0; i < num; ++i) {
        int data = i;
        RID rid = RID(1, 0, i);
        //cout << i << " " << endl;
        indexHandle.InsertEntry((void *)&data, rid);
    }
    //indexHandle->printBPlusTree();

    for (int i = 0; i < num; ++i) {
        int data = i;
        RID rid = RID(1, 0, i);
        //cout << i << " " << endl;
        indexHandle.DeleteEntry((void *)&data, rid);
        //indexHandle->printBPlusTree();
    }
    flag = im->CloseIndex(indexHandle);
    return 0;

    int data1 = 0x3927F85A;
    RID rid1 = RID(1, 0, 0);
    indexHandle.InsertEntry((void *)&data1, rid1);
    cout << "insert entry pData = " << &data1 << " RID = (1, 0, 0)" << endl;
    int data2 = 0x28408F21;
    RID rid2 = RID(1, 0, 1);
    indexHandle.InsertEntry((void *)&data2, rid2);
    cout << "insert entry pData = " << &data2 << " RID = (1, 0, 1)" << endl;

    flag = indexHandle.DeleteEntry((void *)&data1, rid1);
    if (flag) {
        cout << "delete entry successfully  pData = " << &data1 << " RID = (1, 0, 0)" << endl;
    }
    flag = indexHandle.DeleteEntry((void *)&data2, rid1);
    if (!flag) {
        cout << "delete entry unsuccessfully  pData = " << &data2 << " RID = (1, 0, 0)" << endl;
    }
    // flag = indexHandle->deleteEntry((void *)&data2, rid2);
    // if (flag) {
    //     cout << "delete entry successfully  pData = " << &data2 << " RID = (1, 0, 1)" << endl;
    // }

    flag = im->CloseIndex(indexHandle);
    if (flag) {
        cout << "close index successfully" << endl;
    }

    flag = im->OpenIndex("test", 0, indexHandle);
    if (flag) {
        cout << "reopen index successfully" << endl;
    }
    

/*
    IX_IndexScan indexScan;
    flag = indexScan.openScan(indexHandle, NO_OP, NULL);
    if (flag) {
        cout << "open scan successfully" << endl;
    }

    RID rid;
    flag = indexScan.getNextEntry(rid);
    if (flag) {
        cout << "get next entry successfully" << endl;
    }

    cout << "RID = " << rid.fileID << " " << rid.pageID << " " << rid.slotID << endl;

    flag = im->destroyIndex("test", 0);
    if (flag) {
        cout << "destroy index successfully" << endl;
    }
*/
    delete fm;
    delete bpm;
    delete im;
    return 0;
}
