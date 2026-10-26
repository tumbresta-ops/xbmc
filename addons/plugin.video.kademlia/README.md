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

Kademlia DHT must be enabled in Kodi's Services settings. Each catalog item
points directly to a `magnet://?xt=...` folder. Selecting a movie opens the
torrent contents immediately, without an extra plugin or folder-selection
step. Select a video file inside that folder to play it; Kodi opens it directly
from its `magnet://<torrent path>?xt=...` URL.

These public sample magnets use BitTorrent v1 info hashes. Kodi must be built
with the Kademlia v1/v2 hash-key fix; older builds that key all torrents by
the v2 hash can mistakenly reuse Big Buck Bunny metadata for other v1 torrents.

Kodi's Kademlia storage keeps torrent metadata on disk, capped at 50 `.torrent`
files, and uses temporary memory storage for streamed pieces. Stream data is
released when the file closes; while playback is active, memory use grows with
the pieces downloaded for the selected video.

A specific magnet can also be passed to the add-on through a `magnet` query
parameter, for example:

```text
plugin://plugin.video.kademlia/?magnet=magnet%3A%3Fxt%3Durn%3Abtih%3Add8255ecdc7ca55fb0bbf81323d87062db1f6d1c%26dn%3DBig%2520Buck%2520Bunny
```

JSON-RPC callers can also pass the magnet in the `action` parameter:

```text
plugin://plugin.video.kademlia/?action=magnet:magnet:?xt=urn:btih:dd8255ecdc7ca55fb0bbf81323d87062db1f6d1c&dn=Big%20Buck%20Bunny
```

The add-on turns that request into the same native `magnet://` VFS folder.
