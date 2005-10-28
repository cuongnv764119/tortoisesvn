// TortoiseSVN - a Windows shell extension for easy version control

// External Cache Copyright (C) 2005 - Will Dean, Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "StdAfx.h"
#include "SVNStatus.h"
#include "Svnstatuscache.h"
#include "CacheInterface.h"
#include "shlobj.h"

//////////////////////////////////////////////////////////////////////////

CSVNStatusCache* CSVNStatusCache::m_pInstance;

CSVNStatusCache& CSVNStatusCache::Instance()
{
	ATLASSERT(m_pInstance != NULL);
	return *m_pInstance;
}

void CSVNStatusCache::Create()
{
	ATLASSERT(m_pInstance == NULL);
	m_pInstance = new CSVNStatusCache;

	m_pInstance->watcher.SetFolderCrawler(&m_pInstance->m_folderCrawler);
#define LOADVALUEFROMFILE(x) if (fread(&x, sizeof(x), 1, pFile)!=1) goto exit;
#define LOADVALUEFROMFILE2(x) if (fread(&x, sizeof(x), 1, pFile)!=1) goto error;
	unsigned int value = (unsigned int)-1;
	FILE * pFile = NULL;
	// find the location of the cache
	TCHAR path[MAX_PATH];		//MAX_PATH ok here.
	if (SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path)==S_OK)
	{
		_tcscat(path, _T("\\TSVNCache"));
		if (!PathIsDirectory(path))
			CreateDirectory(path, NULL);
		_tcscat(path, _T("\\cache"));
		pFile = _tfopen(path, _T("rb"));
		if (pFile)
		{
			LOADVALUEFROMFILE(value);
			if (value != 0)
			{
				goto exit;
			}
			int mapsize = 0;
			LOADVALUEFROMFILE(mapsize);
			for (int i=0; i<mapsize; ++i)
			{
				LOADVALUEFROMFILE2(value);	
				if (value)
				{
					CString sKey;
					if (fread(sKey.GetBuffer(value), sizeof(TCHAR), value, pFile)!=value)
					{
						sKey.ReleaseBuffer(0);
						goto error;
					}
					sKey.ReleaseBuffer(value);
					CCachedDirectory * cacheddir = new CCachedDirectory();
					if (!cacheddir->LoadFromDisk(pFile))
						goto error;
					m_pInstance->m_directoryCache[CTSVNPath(sKey)] = cacheddir;
					m_pInstance->watcher.AddPath(CTSVNPath(sKey));
				}
			}
		}
	}
exit:
	if (pFile)
		fclose(pFile);
	DeleteFile(path);
	ATLTRACE("cache loaded from disk successfully!\n");
	return;
error:
	fclose(pFile);
	DeleteFile(path);
	delete m_pInstance;
	m_pInstance = new CSVNStatusCache;
	ATLTRACE("cache not loaded from disk\n");
}

bool CSVNStatusCache::SaveCache()
{
#define WRITEVALUETOFILE(x) if (fwrite(&x, sizeof(x), 1, pFile)!=1) goto error;
	unsigned int value = 0;
	// save the cache to disk
	FILE * pFile = NULL;
	// find a location to write the cache to
	TCHAR path[MAX_PATH];		//MAX_PATH ok here.
	if (SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path)==S_OK)
	{
		_tcscat(path, _T("\\TSVNCache"));
		if (!PathIsDirectory(path))
			CreateDirectory(path, NULL);
		_tcscat(path, _T("\\cache"));
		pFile = _tfopen(path, _T("wb"));
		if (pFile)
		{
			value = 0;		// 'version'
			WRITEVALUETOFILE(value);
			value = (int)m_pInstance->m_directoryCache.size();
			WRITEVALUETOFILE(value);
			for (CCachedDirectory::CachedDirMap::iterator I = m_pInstance->m_directoryCache.begin(); I != m_pInstance->m_directoryCache.end(); ++I)
			{
				const CString& key = I->first.GetWinPathString();
				value = key.GetLength();
				WRITEVALUETOFILE(value);
				if (value)
				{
					if (fwrite(key, sizeof(TCHAR), value, pFile)!=value)
						goto error;
					if (!I->second->SaveToDisk(pFile))
						goto error;
				}
			}
			fclose(pFile);
		}
	}
	ATLTRACE("cache saved to disk at %ws\n", path);
	return true;
error:
	fclose(pFile);
	delete m_pInstance;
	m_pInstance = NULL;
	return false;
}

void CSVNStatusCache::Destroy()
{
	if (m_pInstance)
		m_pInstance->Stop();
	delete m_pInstance;
	m_pInstance = NULL;
}

void CSVNStatusCache::Stop()
{
	watcher.Stop();
	m_folderCrawler.Stop();
	m_shellUpdater.Stop();
}

CSVNStatusCache::CSVNStatusCache(void)
{
	m_folderCrawler.Initialise();
	m_shellUpdater.Initialise();
}

CSVNStatusCache::~CSVNStatusCache(void)
{
	for (CCachedDirectory::CachedDirMap::iterator I = m_pInstance->m_directoryCache.begin(); I != m_pInstance->m_directoryCache.end(); ++I)
	{
		delete I->second;
	}
}

void CSVNStatusCache::Clear()
{

}

void CSVNStatusCache::UpdateShell(const CTSVNPath& path)
{
	m_shellUpdater.AddPathForUpdate(path);
}

void CSVNStatusCache::RemoveCacheForPath(const CTSVNPath& path)
{
	// Stop the crawler starting on a new folder
	CCrawlInhibitor crawlInhibit(&m_folderCrawler);
	AutoLocker lock(m_critSec);
	CCachedDirectory * dirtoremove = m_directoryCache[path];
	delete dirtoremove;
	m_directoryCache.erase(path);
	ATLTRACE("removed path %ws from cache\n", path.GetWinPath());
}

CCachedDirectory * CSVNStatusCache::GetDirectoryCacheEntry(const CTSVNPath& path)
{
	ATLASSERT(path.IsDirectory() || !PathFileExists(path.GetWinPath()));

	AutoLocker lock(m_critSec);

	CCachedDirectory::ItDir itMap;
	itMap = m_directoryCache.find(path);
	if(itMap != m_directoryCache.end())
	{
		// We've found this directory in the cache 
		return itMap->second;
	}
	else
	{
		// We don't know anything about this directory yet - lets add it to our cache
		CCachedDirectory * cdir = m_directoryCache.insert(m_directoryCache.lower_bound(path), std::make_pair(path, new CCachedDirectory(path)))->second;
		if (!path.IsEmpty())
			watcher.AddPath(path);
		return cdir;		
	}
}


CStatusCacheEntry CSVNStatusCache::GetStatusForPath(const CTSVNPath& path, DWORD flags)
{
	bool bRecursive = !!(flags & TSVNCACHE_FLAGS_RECUSIVE_STATUS);

	// Check a very short-lived 'mini-cache' of the last thing we were asked for.
	long now = (long)GetTickCount();
	if(now-m_mostRecentExpiresAt < 0)
	{
		if(path.IsEquivalentTo(m_mostRecentPath))
		{
			return m_mostRecentStatus;
		}
	}
	m_mostRecentPath = path;
	m_mostRecentExpiresAt = now+1000;

	// Stop the crawler starting on a new folder while we're doing this much more important task...
	// Please note, that this may be a second "lock" used concurrently to the one in RemoveCacheForPath().
	CCrawlInhibitor crawlInhibit(&m_folderCrawler);

	return m_mostRecentStatus = GetDirectoryCacheEntry(path.GetContainingDirectory())->GetStatusForMember(path, bRecursive);
}

void CSVNStatusCache::AddFolderForCrawling(const CTSVNPath& path)
{
	m_folderCrawler.AddDirectoryForUpdate(path);
}

//////////////////////////////////////////////////////////////////////////

static class StatusCacheTests
{
public:
	StatusCacheTests()
	{
		//apr_initialize();
		//CSVNStatusCache::Create();
		//{
		//	CSVNStatusCache& cache = CSVNStatusCache::Instance();

		//	cache.GetStatusForPath(CTSVNPath(_T("D:/Development/SVN/TortoiseSVN")), TSVNCACHE_FLAGS_RECUSIVE_STATUS);
		//	cache.GetStatusForPath(CTSVNPath(_T("D:/Development/SVN/Subversion")), TSVNCACHE_FLAGS_RECUSIVE_STATUS);
		//}

		//CSVNStatusCache::Destroy();
		//apr_terminate();
	}

} StatusCacheTests;


