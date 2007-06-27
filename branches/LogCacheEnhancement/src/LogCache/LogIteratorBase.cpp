// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2007-2007 - TortoiseSVN

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "StdAfx.h"
#include "LogIteratorBase.h"

// begin namespace LogCache

namespace LogCache
{

// react on cache updates

void CLogIteratorBase::HandleCacheUpdates()
{
	// maybe, we can now use a shorter relative path

	path.RepeatLookup();
}

// comparison methods

bool CLogIteratorBase::PathsIntersect ( const CDictionaryBasedPath& lhsPath
									  , const CDictionaryBasedPath& rhsPath)
{
	return lhsPath.IsSameOrParentOf (rhsPath) 
		|| rhsPath.IsSameOrParentOf (lhsPath);
}

// is current revision actually relevant?

bool CLogIteratorBase::PathInRevision
	( const CRevisionInfoContainer::CChangesIterator& first
	, const CRevisionInfoContainer::CChangesIterator& last
	, const CDictionaryBasedTempPath& path)
{
	// close examination of all changes

	for ( CRevisionInfoContainer::CChangesIterator iter = first
		; iter != last
		; ++iter)
	{
		// if (and only if) path is a cached path, 
		// it may be a parent of the changedPath
		// (i.e. report a change of this or some sub-path)

		CDictionaryBasedPath changedPath = iter->GetPath();
		if (   path.IsFullyCachedPath() 
			&& path.GetBasePath().IsSameOrParentOf (changedPath))
			return true;

		// this change affects a true parent path or completly unrelated path
		// -> ignore mere modifications (e.g. properties on a folder)

		if (iter->GetAction() == CRevisionInfoContainer::ACTION_CHANGED)
			continue;

		// this is an add / delete / replace.
		// does it affect our path?

		if (changedPath.IsSameOrParentOf (path.GetBasePath()))
			return true;
	}

	// no paths that we were looking for

	return false;
}

bool CLogIteratorBase::PathInRevision() const
{
	assert (!InternalDataIsMissing());

	// special case: repository root 
	// (report all revisions including empty ones)

	if (path.IsRoot())
		return true;

	// revision data lookup

	const CRevisionInfoContainer& revisionInfo = logInfo->GetLogInfo();
	revision_t index = logInfo->GetRevisions()[revision];

	// any chance that this revision affects our path?

	CDictionaryBasedPath revisionRootPath = revisionInfo.GetRootPath (index);
	if (!revisionRootPath.IsValid())
		return false;

	if (!PathsIntersect (path.GetBasePath(), revisionRootPath))
		return false;

	// close examination of all changes

	return PathInRevision ( revisionInfo.GetChangesBegin(index)
						  , revisionInfo.GetChangesEnd(index)
						  , path);
}

// Test, whether InternalHandleCopyAndDelete() should be used

bool CLogIteratorBase::ContainsCopyOrDelete 
	( const CRevisionInfoContainer::CChangesIterator& first
	, const CRevisionInfoContainer::CChangesIterator& last)
{
	// close examination of all changes

	for (CRevisionInfoContainer::CChangesIterator iter = first
		; iter != last
		; ++iter)
	{
		// the only non-critical operation is the mere modification

		CRevisionInfoContainer::TChangeAction action = iter.GetAction();
		if (action != CRevisionInfoContainer::ACTION_CHANGED)
			return true;
	}

	// no copy / delete / replace found

	return false;
}

// Change the path we are iterating the log for,
// if there is a copy / replace.
// Set revision to NO_REVISION, if path is deleted.

bool CLogIteratorBase::InternalHandleCopyAndDelete 
	( const CRevisionInfoContainer::CChangesIterator& first
	, const CRevisionInfoContainer::CChangesIterator& last
	, const CDictionaryBasedPath& revisionRootPath
	, CDictionaryBasedTempPath& searchPath
	, revision_t& searchRevision)
{
	bool addFound = false;
	// any chance that this revision affects our search path?

	if (!revisionRootPath.IsValid())
		return false;

	if (!revisionRootPath.IsSameOrParentOf (searchPath.GetBasePath()))
		return false;

	// close examination of all changes

	for ( CRevisionInfoContainer::CChangesIterator iter = first
		; iter != last
		; ++iter)
	{
		// most entries will just be file content changes
		// -> skip them efficiently

		CRevisionInfoContainer::TChangeAction action = iter.GetAction();
		if (action == CRevisionInfoContainer::ACTION_CHANGED)
			continue;

		// deletion / copy / rename / replacement
		// -> skip, if our search path is not affected (only some sub-path)

		CDictionaryBasedPath changedPath = iter->GetPath();
		if (!changedPath.IsSameOrParentOf (searchPath.GetBasePath()))
			continue;

		// now, this is serious

		switch (action)
		{
			// rename?

			case CRevisionInfoContainer::ACTION_ADDED:
			case CRevisionInfoContainer::ACTION_REPLACED:
			{
				if (iter.HasFromPath())
				{
					// continue search on copy source path

					searchPath = searchPath.ReplaceParent ( iter.GetPath()
														  , iter.GetFromPath());
					searchRevision = iter.GetFromRevision();
					return true;
				}
				else
				{
					searchRevision = NO_REVISION;
					addFound = true;
					// we don't return here immediately, since an ADD without
					// a copyfrom doesn't mean necessarily that our path got
					// added in this revision. Instead, just set a flag indicating
					// that this method should return true if we don't find
					// another real rename of our path.
					//
					// example:
					//
					// our path: /trunk/file
					// renamed to
					// /trunk/project/file
					// 
					// this can only happen if
					// /trunk/project
					// is added first (usually without a copyfrom path)
				}
			}
			break;

			case CRevisionInfoContainer::ACTION_DELETED:
			{
				// deletions are possible!
				// but we don't need to do anything with them.
			}
			break;

			// there should be no other

			default:
			{
				assert (0);
			}
		}
	}

	if (addFound)
		return true;

	// all fine, no special action required

	return false;
}

// log scanning sub-routines

void CLogIteratorBase::ToNextRevision()
{
	--revision;
}

revision_t CLogIteratorBase::SkipNARevisions()
{
	return logInfo->GetSkippedRevisions()
		.GetPreviousRevision (path.GetBasePath(), revision);
}

// log scanning

void CLogIteratorBase::InternalAdvance()
{
	// find next entry that mentiones the path
	// stop @ revision 0 or missing log data

	do
	{
		// perform at least one step

		ToNextRevision();

		// skip ranges of missing data, if we know
		// that they don't affect our path

		while (InternalDataIsMissing())
		{
			revision_t nextRevision = SkipNARevisions(); 
			if (nextRevision != NO_REVISION)
				revision = nextRevision;
			else
				break;
		}
	}
	while ((revision > 0) && !InternalDataIsMissing() && !PathInRevision());
}

// construction 
// (copy construction & assignment use default methods)

CLogIteratorBase::CLogIteratorBase ( const CCachedLogInfo* cachedLog
								   , revision_t startRevision
								   , const CDictionaryBasedTempPath& startPath)
	: logInfo (cachedLog)
	, revision (startRevision)
	, path (startPath)
{
}

CLogIteratorBase::~CLogIteratorBase(void)
{
}

// implement ILogIterator

bool CLogIteratorBase::DataIsMissing() const
{
	return InternalDataIsMissing();
}

void CLogIteratorBase::Advance()
{
	// maybe, there was some cache update

	HandleCacheUpdates();

	// end of history?

	if (revision > 0)
	{
		// the current revision may be a copy / rename
		// -> update our path before we proceed, if necessary

		if (HandleCopyAndDelete())
		{
			// revision may have been set to NO_REVISION, 
			// e.g. if a deletion has been found

			if (revision != NO_REVISION)
			{
				// switched to a new path
				// -> retry access on that new path 
				// (especially, we must try (copy-from-) revision)

				Retry();
			}
		}
		else
		{
			// find next entry that mentiones the path
			// stop @ revision 0 or missing log data

			InternalAdvance();
		}
	}
}

void CLogIteratorBase::Retry()
{
	// maybe, there was some cache update

	HandleCacheUpdates();

	// don't handle copy / rename more than once

	++revision;
	InternalAdvance();
}

// end namespace LogCache

}

