# Kademlia Magnet VFS Test Browser

This Kodi video add-on hands the torrent directly to Kodi's `magnet://` VFS.
The add-on does not enumerate torrent contents in Python. Kodi opens the VFS
folder and handles its files like a native directory.

Open the add-on under **Video add-ons**, choose a test movie, open its torrent
folder, and select a video file. The catalog includes these Blender Open
Movies, which are published for free public viewing:

```text
Big Buck Bunny  https://peach.blender.org/
Sintel          https://durian.blender.org/
Tears of Steel  https://mango.blender.org/
```

Kademlia DHT must be enabled in Kodi's Services settings. The plugin URL
creates a catalog of folder items whose targets are `magnet://?xt=...`;
selecting one enters Kodi's native torrent browser. Video files inside the
torrent are then opened directly by Kodi from their
`magnet://<torrent path>?xt=...` URLs.

These public sample magnets use BitTorrent v1 info hashes. Kodi must be built
with the Kademlia v1/v2 hash-key fix; older builds that key all torrents by
the v2 hash can mistakenly reuse Big Buck Bunny metadata for other v1 torrents.

A specific magnet can still be opened directly with a plugin URL, for example:

```text
plugin://plugin.video.kademlia/magnet:?xt=urn:btih:dd8255ecdc7ca55fb0bbf81323d87062db1f6d1c&dn=Big%20Buck%20Bunny
```

JSON-RPC callers can also pass the magnet in the `action` parameter:

```text
plugin://plugin.video.kademlia/?action=magnet:magnet:?xt=urn:btih:dd8255ecdc7ca55fb0bbf81323d87062db1f6d1c&dn=Big%20Buck%20Bunny
```

The add-on turns that request into the same native `magnet://` VFS folder.
