# Kademlia Magnet Player

This Kodi video add-on provides a Big Buck Bunny magnet link for testing a Kodi
build with Kademlia support.

Open the add-on under **Video add-ons** and select **Big Buck Bunny (Kademlia
test)**, or call Kodi JSON-RPC with:

```json
{
  "jsonrpc": "2.0",
  "method": "Player.Open",
  "params": {
    "item": {
      "file": "plugin://plugin.video.kademlia/?action=play"
    }
  },
  "id": 1
}
```

Kademlia DHT must be enabled in Kodi's Services settings. The magnet URI can
also be opened directly with `Player.Open`; this add-on is a convenient
playback test entry point.
