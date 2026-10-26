/*
 *  Copyright (C) 2023 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include <mutex>
#include <string>

#include <libtorrent/libtorrent.hpp>

namespace KODI
{
namespace NETWORK
{
/*!
 * \brief Subsystem for handling libtorrent caching
 */
class CLibtorrentCache
{
public:
  CLibtorrentCache();
  ~CLibtorrentCache() = default;

  const std::string& GetCachePath() const { return m_cachePath; }

  std::string GetMetadataPath(const lt::info_hash_t& infoHash) const;

  void SaveMetadata(const lt::add_torrent_params& addTorrentParams);

private:
  void CreatePaths();
  void RemovePersistedPayload();
  void PruneMetadata();

  const std::string m_cachePath;
  std::mutex m_cacheMutex;
};
} // namespace NETWORK
} // namespace KODI
