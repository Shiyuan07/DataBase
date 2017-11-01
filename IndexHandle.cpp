#include <iostream>
#include "IndexHandle.h"

using namespace std;
unsigned char MyBitMap::h[61] = {};

IndexHandle::IndexHandle  (BufPageManager *bufPageManager, int fileID)
{
  mBufPageManager = bufPageManager;
  mFileID = fileID;
  mTreeHeader = (IndexTreeHeader*)(bufPageManager->getPage(fileID, 0, mIndex));
  bufPageManager->access(mIndex);
}

IndexHandle::~IndexHandle() {}

int IndexHandle::getFileID() {
  return mFileID;
}

AttrType IndexHandle::getAttrType() {
  return mTreeHeader->attrType;
}

int IndexHandle::getAttrLength() {
  return mTreeHeader->attrLength;
}

int IndexHandle::getNewPage()
{
  int tmpindex;
	int cache = mTreeHeader->firstFreePage;
  char* newpage = (char*)mBufPageManager->getPage(mFileID, cache, tmpindex);
  mTreeHeader->firstFreePage = *((int*)newpage);
  mTreeHeader->pageNum++;
  mBufPageManager->markDirty(mIndex);
	return cache;
}

int IndexHandle::solveOverFlow(int pageID)
{
	int tmpindex;
	char* page = (char*)mBufPageManager->getPage(mFileID, pageID, tmpindex);
	mBufPageManager->markDirty(tmpindex);
  int newPageID = getNewPage();
	char* newpage = (char*)mBufPageManager->getPage(mFileID, newPageID, tmpindex);
	mBufPageManager->markDirty(tmpindex);

	IndexNodeHeader *nodeheader = (IndexNodeHeader*)page;
	IndexNodeHeader *newnodeheader = (IndexNodeHeader*)newpage;
  newnodeheader->num_keys = nodeheader-> num_keys/2;
  newnodeheader->isLeafNode = nodeheader->isLeafNode;
	if (nodeheader->isLeafNode == true)
	{
		nodeheader->num_keys -= newnodeheader->num_keys;
		RID *rid = calcRid(page, nodeheader->num_keys);
		memcpy(newpage + sizeof(IndexNodeHeader), rid, calcLength(nodeheader->num_keys));
		*rid = RID(mFileID, newPageID, -1);
	}
	else
	{
		nodeheader->num_keys -= newnodeheader->num_keys+1;
		memcpy(newpage + sizeof(IndexNodeHeader), calcRid(page, nodeheader->num_keys + 1), (newnodeheader->num_keys + 1) * (sizeof(RID) + mTreeHeader -> attrLength));
	}
  return newPageID;
}


void IndexHandle::insertNode(void *pData, const RID &rid, char* insertpage, int slot)
{
	IndexNodeHeader *nodeheader = (IndexNodeHeader*)insertpage;
	RID *rid1 = calcRid(insertpage, nodeheader->num_keys + 1);
	RID *rid2 = calcRid(insertpage, nodeheader->num_keys);
	*rid1 = (*rid2);
	for (int i = (nodeheader->num_keys) - 1; i >= slot; i--)
	{
		rid1 = calcRid(insertpage, i + 1);
		rid2 = calcRid(insertpage, i);
		*rid1 = (*rid2);
		char *dst = insertpage + calcLength(i + 1);
    char *src = insertpage + calcLength(i);
		setKey(dst, src);
	}
	nodeheader -> num_keys++;
	rid1 = calcRid(insertpage, slot);
	*rid1 = rid;
  char *dst = insertpage + calcLength(slot);
	setKey(dst, pData);
}

int IndexHandle::findSlot(int nums, char* page, void *pData)
{
  int slot = nums;
  for (int i = 0; i < nums; i++)
  {
    char *cmpData = page + calcLength(i);
    if (compareData(pData, cmpData))
    {
      slot = i;
      break;
    }
  }
  return slot;
}

bool IndexHandle::insertUpdate(int pageID, void *pData, const RID &rid)
{
	int index;
	char* page = (char*)mBufPageManager->getPage(mFileID, pageID, index);
	mBufPageManager->access(index);
	IndexNodeHeader *nodeheader = (IndexNodeHeader*)page;
	if (nodeheader->isLeafNode == true) // leaf
	{
		int slot = findSlot(nodeheader->num_keys, page, pData);
		insertNode(pData, rid, page, slot);
		mBufPageManager->markDirty(index);
		if (nodeheader->num_keys > mTreeHeader->maxN)
			return true;
	}
	else // node
	{
    int slot = findSlot(nodeheader->num_keys, page, pData);
		RID *r = calcRid(page, slot);
    int tmpfileid, nextpageid;
    r -> getPageNum(tmpfileid, nextpageid);
		if (insertUpdate(nextpageid, pData, rid))
		{
			int newpage = solveOverFlow(nextpageid);
			RID newr = RID(mFileID, newpage, -1);
      char* insertdata;
      int tmpindex;
    	char* nextpage = (char*)mBufPageManager->getPage(mFileID, nextpageid, tmpindex);
    	mBufPageManager->access(tmpindex);
    	IndexNodeHeader* nextnodeheader = (IndexNodeHeader*)mBufPageManager;
      if (nextnodeheader->isLeafNode == false)
        insertdata = nextpage + calcLength(nextnodeheader->num_keys);
    	else
    		insertdata = nextpage + calcLength(nextnodeheader->num_keys - 1);
			insertNode(insertdata, *r, page, slot);
      r = calcRid(page, slot + 1);
			*r = newr;
			mBufPageManager->markDirty(index);
			if (nodeheader->num_keys > mTreeHeader->maxN)
				return true;
		}
	}
	return false;
}


int IndexHandle::InsertEntry(void *pData, const RID &rid)
{
	mBufPageManager->access(mIndex);

	if (mTreeHeader->rootPage == -1)
	{
		mTreeHeader->rootPage = getNewPage();
		int tmpindex;
		char* page = (char*)mBufPageManager->getPage(mFileID, mTreeHeader->rootPage, tmpindex);
		mBufPageManager->markDirty(tmpindex);
		IndexNodeHeader *nodeheader = (IndexNodeHeader*)page;
		nodeheader->num_keys = 1;
		nodeheader->isLeafNode = true;
		RID* r = calcRid(page, 0);
		(*r) = rid;
    char *dst = page + calcLength(0);
		setKey(dst, pData);
		r = calcRid(page, 1);
		(*r) = RID(mFileID, -1, -1);
	}
	else
	{
		if (insertUpdate(mTreeHeader->rootPage, pData, rid) == true)
		{
			int newPageID = solveOverFlow(mTreeHeader->rootPage);
			int newrootpage = getNewPage();
			int tmpindex;
			char* page = (char*)mBufPageManager->getPage(mFileID, newrootpage, tmpindex);
			mBufPageManager->markDirty(tmpindex);

			IndexNodeHeader *nodeheader = (IndexNodeHeader*)page;
			nodeheader->num_keys = 1;
			nodeheader->isLeafNode = false;
			RID *r = calcRid(page, 0);
			(*r) = RID(mFileID, mTreeHeader->rootPage, -1);
			r = calcRid(page, 1);
			(*r) = RID(mFileID, newPageID, -1);
      char *dst = page + calcLength(0);

    	page = (char*)mBufPageManager->getPage(mFileID, mTreeHeader->rootPage, tmpindex);
    	mBufPageManager->access(tmpindex);
    	nodeheader = (IndexNodeHeader*)page;
      char *src;
    	if (nodeheader->isLeafNode == false)
        src = page + calcLength(nodeheader->num_keys);
    	else
    		src = page + calcLength(nodeheader->num_keys - 1);
			setKey(dst, src);
			mTreeHeader->rootPage = newrootpage;
		}
	}
	return 1;
}

bool IndexHandle::DeleteEntry (void *pData, const RID &rid)
{
  mBufPageManager->access(mIndex);
  if (mTreeHeader->rootPage == -1)
		return false;
	return DeleteFromNode(mTreeHeader->rootPage, pData, rid);
}

bool IndexHandle::DeleteFromNode(int pageID, void *pData, const RID &rid)
{
	int index;
	char* page = (char*)mBufPageManager->getPage(mFileID, pageID, index);
	mBufPageManager->access(index);
	IndexNodeHeader *nodeheader = (IndexNodeHeader*)page;
	if (nodeheader->isLeafNode == true)
	{
    return DeleteFromLeaf(pData, rid, page, index);
	}

  int slot = findSlot(nodeheader->num_keys, page, pData);
  RID *r = calcRid(page, slot);
  int tmpfileid, nextpageid;
  r -> getPageNum(tmpfileid, nextpageid);
  return DeleteFromNode(nextpageid, pData, rid);
}
bool IndexHandle::DeleteFromLeaf(void *pData, const RID &rid, char* page, int index)
{
  IndexNodeHeader *nodeheader = (IndexNodeHeader*)page;
  while (true)
  {
    for (int i = 0; i < nodeheader->num_keys; i++)
    {
      char *cmpData = page + calcLength(i);
      if (compareData(pData, cmpData))
      {
        if (equalData(pData, cmpData) == false)
          return false;
        RID *r = calcRid(page, i);
        if (rid == (*r))
        {
          deleteNode(pData, rid, page, i);
          mBufPageManager->markDirty(index);
          return true;
        }
      }
    }
    RID *r = calcRid(page, nodeheader->num_keys);
    int tmpfileid, nextpageid;
    r -> getPageNum(tmpfileid, nextpageid);
    if (nextpageid == -1)
      return false;
    page = (char*)mBufPageManager->getPage(mFileID, nextpageid, index);
    mBufPageManager->access(index);
    nodeheader = (IndexNodeHeader*)page;
  }
}

void IndexHandle::deleteNode(void *pData, const RID &rid, char* deletepage, int slot)
{
	IndexNodeHeader *nodeheader = (IndexNodeHeader*)deletepage;
	for (int i = slot; i < nodeheader->num_keys; i++)
	{
		RID *rid1 = calcRid(deletepage, i);
		RID *rid2 = calcRid(deletepage, i + 1);
		*rid1 = (*rid2);
    char *dst = deletepage + calcLength(i);
    char *src = deletepage + calcLength(i+1);
		setKey(dst, src);
	}
	nodeheader->num_keys--;
}

int IndexHandle::ForcePages()
{
	mBufPageManager->close();
	return 0;
}
