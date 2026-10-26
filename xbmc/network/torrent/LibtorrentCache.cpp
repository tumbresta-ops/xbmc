/*
 *  Copyright (C) 2023 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "LibtorrentCache.h"

#include "TorrentUtils.h"
#include "URL.h"
#include "filesystem/Directory.h"
#include "filesystem/File.h"
#include "FileItem.h"
#include "filesystem/FileItemList.h"
#include "filesystem/SpecialProtocol.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/log.h"

#include <algorithm>
#include <sstream>
#include <vector>

using namespace KODI;
using namespace NETWORK;

CLibtorrentCache::CLibtorrentCache()
  : m_cachePath(CSpecialProtocol::TranslatePath("special://home/cache/kademlia"))
{
  CreatePaths();
  RemovePersistedPayload();
  PruneMetadata();
}

std::string CLibtorrentCache::GetMetadataPath(const lt::info_hash_t& infoHash) const
{
  std::stringstream str;
  str << CTorrentUtils::GetCanonicalInfoHash(infoHash);

  return URIUtils::AddFileToFolder(m_cachePath, str.str() + ".torrent");
}

void CLibtorrentCache::SaveMetadata(const lt::add_torrent_params& addTorrentParams)
{
  std::lock_guard lock(m_cacheMutex);

  const std::string torrentPath = GetMetadataPath(addTorrentParams.info_hashes);

  // Encode metadata
  std::vector<char> buffer;
  try
  {
    buffer = lt::write_torrent_file_buf(addTorrentParams, {});
  }
  catch (const std::exception& e)
  {
    CLog::Log(LOGERROR, "Failed to encode torrent file: {}", e.what());
    return;
  }

  // Write metadata to cache
  XFILE::CFile file;
  if (!file.OpenForWrite(torrentPath, true))
  {
    CLog::Log(LOGERROR, "Failed to open torrent file for writing: {}",
              CURL::GetRedacted(torrentPath));
    return;
  }

  const auto bytesWritten = file.Write(buffer.data(), buffer.size());
  if (bytesWritten < 0 || static_cast<size_t>(bytesWritten) != buffer.size())
  {
    CLog::Log(LOGERROR, "Failed to write torrent metadata: {}", CURL::GetRedacted(torrentPath));
    return;
  }

  file.Close();
  PruneMetadata();
}

void CLibtorrentCache::PruneMetadata()
{
  constexpr size_t MAX_METADATA_FILES = 50;

  CFileItemList items;
  if (!XFILE::CDirectory::GetDirectory(m_cachePath, items, ".torrent"))
  {
    CLog::Log(LOGERROR, "Failed to list Kademlia metadata cache");
    return;
  }

  std::vector<CFileItemPtr> metadataFiles;
  metadataFiles.reserve(items.Size());
  for (const CFileItemPtr& item : items)
  {
    if (!item->IsFolder())
      metadataFiles.emplace_back(item);
  }

  std::sort(metadataFiles.begin(), metadataFiles.end(),
            [](const CFileItemPtr& lhs, const CFileItemPtr& rhs)
            {
              const CDateTime& lhsTime = lhs->GetDateTime();
              const CDateTime& rhsTime = rhs->GetDateTime();
              if (lhsTime.IsValid() != rhsTime.IsValid())
                return !lhsTime.IsValid();
              if (lhsTime.IsValid())
              {
                const std::string lhsDate = lhsTime.GetAsDBDateTime();
                const std::string rhsDate = rhsTime.GetAsDBDateTime();
                if (lhsDate != rhsDate)
                  return lhsDate < rhsDate;
              }
              return lhs->GetPath() < rhs->GetPath();
            });

  while (metadataFiles.size() > MAX_METADATA_FILES)
  {
    const std::string path = metadataFiles.front()->GetPath();
    if (!XFILE::CFile::Delete(path))
    {
      CLog::Log(LOGERROR, "Failed to remove old torrent metadata: {}", CURL::GetRedacted(path));
      return;
    }
    metadataFiles.erase(metadataFiles.begin());
  }
}

void CLibtorrentCache::RemovePersistedPayload()
{
  CFileItemList items;
  if (!XFILE::CDirectory::GetDirectory(m_cachePath, items, "", XFILE::DIR_FLAG_NO_FILE_DIRS))
  {
    CLog::Log(LOGERROR, "Failed to list Kademlia cache while removing old torrent payload");
    return;
  }

  for (const CFileItemPtr& item : items)
  {
    if (!item->IsFolder() &&
        StringUtils::EqualsNoCase(URIUtils::GetExtension(item->GetPath()), ".torrent"))
    {
      continue;
    }

    const bool removed = item->IsFolder() ? XFILE::CDirectory::Remove(item->GetPath())
                                          : XFILE::CFile::Delete(item->GetPath());
    if (!removed)
    {
      CLog::Log(LOGERROR, "Failed to remove persisted torrent payload: {}",
                CURL::GetRedacted(item->GetPath()));
    }
  }
}

void CLibtorrentCache::CreatePaths()
{
  // Create folder if it doesn't exist
  if (!XFILE::CDirectory::Exists(m_cachePath))
  {
    CLog::Log(LOGDEBUG, "Creating Kademlia cache path: {}", CURL::GetRedacted(m_cachePath));
    if (!XFILE::CDirectory::Create(m_cachePath))
      CLog::Log(LOGERROR, "Failed to create Kademlia cache path");
  }
}
