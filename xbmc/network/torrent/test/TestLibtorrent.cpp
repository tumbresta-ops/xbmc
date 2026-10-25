/*
 *  Copyright (C) 2023 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "network/torrent/Libtorrent.h"
#include "network/torrent/LibtorrentCache.h"
#include "network/torrent/TorrentUtils.h"

#include <memory>

#include <gtest/gtest.h>

using namespace KODI;
using namespace NETWORK;

constexpr const char* TEST_MAGNET_LINK =
    "magnet:?xt=urn:btih:dd8255ecdc7ca55fb0bbf81323d87062db1f6d1c&dn=Big%20Buck%20Bunny";

class TestLibtorrent : public testing::Test
{
protected:
  void SetUp() override
  {
    libtorrent = std::make_unique<CLibtorrent>();
    libtorrent->Start();
  }

  void TearDown() override
  {
    libtorrent->Stop(false);
    libtorrent->Stop(true);
    libtorrent.reset();
  }

  std::unique_ptr<CLibtorrent> libtorrent;
};

TEST_F(TestLibtorrent, IsOnline)
{
  ASSERT_TRUE(libtorrent->IsOnline());
}

TEST_F(TestLibtorrent, V1TorrentMetadataUsesDistinctCacheFiles)
{
  lt::add_torrent_params bigBuckBunny;
  lt::add_torrent_params sintel;
  lt::error_code errorCode;

  lt::parse_magnet_uri(
      "magnet:?xt=urn:btih:dd8255ecdc7ca55fb0bbf81323d87062db1f6d1c&dn=Big%20Buck%20Bunny",
      bigBuckBunny, errorCode);
  ASSERT_FALSE(errorCode);

  lt::parse_magnet_uri("magnet:?xt=urn:btih:08ada5a7a6183aae1e09d831df6748d566095a10&dn=Sintel",
                       sintel, errorCode);
  ASSERT_FALSE(errorCode);

  CLibtorrentCache cache;
  EXPECT_NE(cache.GetMetadataPath(bigBuckBunny.info_hashes),
            cache.GetMetadataPath(sintel.info_hashes));

  const lt::info_hash_t bigBuckBunnyV1(bigBuckBunny.info_hashes.v1);
  EXPECT_EQ(CTorrentUtils::GetCanonicalInfoHash(bigBuckBunny.info_hashes),
            CTorrentUtils::GetCanonicalInfoHash(bigBuckBunnyV1));
}

TEST_F(TestLibtorrent, DISABLED_AddTorrent)
{
  //! @todo: We might need to wait until the DHT is fully bootstrapped before
  //! we can add a torrent
  lt::torrent_handle torrentHandle = libtorrent->AddTorrent(TEST_MAGNET_LINK);
  ASSERT_TRUE(torrentHandle.is_valid());
}
