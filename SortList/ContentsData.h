#pragma once
class SortListData;

class ContentsData
{
public:
	ContentsData(_In_ LPSRow lpsRowData);
	~ContentsData();
	LPSBinary lpEntryID; // Allocated with MAPIAllocateBuffer
	LPSBinary lpLongtermID; // Allocated with MAPIAllocateBuffer
	LPSBinary lpInstanceKey; // Allocated with MAPIAllocateBuffer
	LPSBinary lpServiceUID; // Allocated with MAPIAllocateBuffer
	LPSBinary lpProviderUID; // Allocated with MAPIAllocateBuffer
	wstring m_szDN;
	string m_szProfileDisplayName;
	ULONG ulAttachNum;
	ULONG ulAttachMethod;
	ULONG ulRowID; // for recipients
	ULONG ulRowType; // PR_ROW_TYPE
};