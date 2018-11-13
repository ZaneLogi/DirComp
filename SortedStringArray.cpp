// SortedStringArray.cpp: implementation of the CSortedStringArray class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DirComp.h"
#include "SortedStringArray.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSortedStringArray::CSortedStringArray()
{
    RemoveAll();
}

CSortedStringArray::~CSortedStringArray()
{

}

int CSortedStringArray::AddString(CString &s)
{
    int nRet;
    if( m_nSize == 0) {
        nRet = Add( s);
    }
    else {
        int i = 0;
        for(; i < m_nSize; i++) {
            nRet = s.CompareNoCase(m_pData[i]);
            if( nRet <= 0) break;
        }
        if( nRet < 0 ) {
            InsertAt( i, s);
            nRet = i;
        }
        else if( nRet > 0 ) {
            nRet = Add( s);
        }
    }
    return nRet;
}
