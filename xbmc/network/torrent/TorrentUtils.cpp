/*
 *  Copyright (C) 2023 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "TorrentUtils.h"

#include "URL.h"
#include "utils/StringUtils.h"

using namespace KODI;
using namespace NETWORK;

lt::info_hash_t CTorrentUtils::GetCanonicalInfoHash(const lt::info_hash_t& infoHash)
{
  if (infoHash.has_v1())
    return lt::info_hash_t(infoHash.v1);

  return lt::info_hash_t(infoHash.v2);
}

std::pair<std::string, std::string> CTorrentUtils::SplitMagnetURL(const CURL& url)
{
  std::string filename = url.GetFileName();

  CURL urlCopy(url);
  urlCopy.SetFileName("");
  std::string magnetUri = urlCopy.Get();

  // Remove any protocol slashes from the magnet URI
  if (StringUtils::StartsWith(magnetUri, "magnet://"))
    magnetUri.erase(std::strlen("magnet:"), 2);

  return std::make_pair(magnetUri, filename);
}
