// SortedStringArray.h: interface for the CSortedStringArray class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SORTEDSTRINGARRAY_H__E8CFC024_B7B1_11D3_9F6B_0000E203B5D8__INCLUDED_)
#define AFX_SORTEDSTRINGARRAY_H__E8CFC024_B7B1_11D3_9F6B_0000E203B5D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSortedStringArray : public CStringArray
{
public:
    int AddString(CString& s);
    CSortedStringArray();
    virtual ~CSortedStringArray();

};

#endif // !defined(AFX_SORTEDSTRINGARRAY_H__E8CFC024_B7B1_11D3_9F6B_0000E203B5D8__INCLUDED_)
