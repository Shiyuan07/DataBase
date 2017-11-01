#ifndef IndexManager_h
#define IndexManager_h
#include "IndexHandle.h"

class IndexManager {
  public:
       IndexManager   (FileManager *fm, BufPageManager* bpm);              // Constructor
       ~IndexManager  ();                             // Destructor
    bool CreateIndex  (const char *fileName,          // Create new index
                     int        indexNo,
                     AttrType   attrType,
                     int        attrLength);
    bool DestroyIndex (const char *fileName,          // Destroy index
                     int        indexNo);
    bool OpenIndex    (const char *fileName,          // Open index
                     int        indexNo,
                     IndexHandle &indexHandle);
    bool CloseIndex   (IndexHandle &indexHandle);  // Close index

  private:
    FileManager *mFileManager;
    BufPageManager *mBufPageManager;
    char* makeFileName(const char *fileName, int indexNo);
};

#endif
