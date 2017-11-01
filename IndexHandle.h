#ifndef IndexHandle_h
#define IndexHandle_h

#include "Record.h"
#include "info.h"
#include "fileio/FileManager.h"
#include "bufmanager/BufPageManager.h"

struct IndexTreeHeader {
  int firstFreePage;
	int rootPage;
  int pageNum;
  AttrType attrType;
	int attrLength;
  int maxN;
};

struct IndexNodeHeader {
  bool isLeafNode;  // indicator for whether it is a leaf node
  int num_keys;     // number of valid keys the node holds
};

class IndexHandle {
  public:
       IndexHandle  () {}
       IndexHandle  (BufPageManager *bufPageManager, int fileID);                            // Constructor
       ~IndexHandle ();                             // Destructor
  int InsertEntry     (void *pData, const RID &rid);  // Insert new index entry
  bool DeleteEntry     (void *pData, const RID &rid);  // Delete index entry
  int ForcePages      ();                            // Copy index to disk
  int getFileID     ();
  AttrType getAttrType();
  int getAttrLength();
  private:
    BufPageManager *mBufPageManager;
    int mFileID;
    IndexTreeHeader *mTreeHeader;
    int mIndex;

    RID *calcRid(char* page, int num) {
      return (RID*)(page + sizeof(IndexNodeHeader) + num * (sizeof(RID) + mTreeHeader -> attrLength));
    }
    int calcLength(int num){
      return sizeof(IndexNodeHeader) + num * (sizeof(RID) + mTreeHeader -> attrLength) + sizeof(RID);
    }

    void setKey(char *dst, void *pData) {
      memcpy(dst, pData, mTreeHeader->attrLength);
    }

    bool compareData(void *pData, char *cmpData) {
      if(mTreeHeader->attrType == INTEGER) {
        int data1 = *((int*)pData);
        int data2 = *((int*)cmpData);
        return data1 <= data2;
      }
      else if(mTreeHeader->attrType == FLOAT) {
        float data1 = *((int*)pData);
        float data2 = *((int*)cmpData);
        return data1 <= data2;
      }
      else if(mTreeHeader->attrType == CHAR) {
    		char* data1 = new char[mTreeHeader->attrLength + 1];
    		memcpy(data1, pData, mTreeHeader->attrLength);
    		data1[mTreeHeader->attrLength] = 0;
        char* data2 = new char[mTreeHeader->attrLength + 1];
    		memcpy(data2, cmpData, mTreeHeader->attrLength);
    		data2[mTreeHeader->attrLength] = 0;
        bool res = (strcmp(data1, data2) <= 0);
        delete data1, data2;
        return res;
      }
    }

    bool equalData(void *pData, char *cmpData) {
      if(mTreeHeader->attrType == INTEGER) {
        int data1 = *((int*)pData);
        int data2 = *((int*)cmpData);
        return data1 == data2;
      }
      else if(mTreeHeader->attrType == FLOAT) {
        float data1 = *((int*)pData);
        float data2 = *((int*)cmpData);
        return data1 == data2;
      }
      else if(mTreeHeader->attrType == CHAR) {
    		char* data1 = new char[mTreeHeader->attrLength + 1];
    		memcpy(data1, pData, mTreeHeader->attrLength);
    		data1[mTreeHeader->attrLength] = 0;
        char* data2 = new char[mTreeHeader->attrLength + 1];
    		memcpy(data2, cmpData, mTreeHeader->attrLength);
    		data2[mTreeHeader->attrLength] = 0;
        bool res = (strcmp(data1, data2) == 0);
        delete data1, data2;
        return res;
      }
    }


    int getNewPage();
    int solveOverFlow(int pageID);


    void insertNode(void *pData, const RID &rid, char* insertpage, int slot);
    bool insertUpdate(int pageID, void *pData, const RID &rid);
    int findSlot(int nums, char* page, void *pData);
    bool DeleteFromNode(int pageID, void *pData, const RID &rid);
    bool DeleteFromLeaf(void *pData, const RID &rid, char* page, int index);
    void deleteNode(void *pData, const RID &rid, char* deletepage, int slot);


 };

#endif
