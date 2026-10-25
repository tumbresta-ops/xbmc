import sys
from urllib.parse import parse_qs, urlencode

import xbmcgui
import xbmcplugin


PLUGIN_URL = sys.argv[0]
PLUGIN_HANDLE = int(sys.argv[1])
MAGNET_URI = (
    "magnet:?xt=urn:btih:dd8255ecdc7ca55fb0bbf81323d87062db1f6d1c"
    "&dn=Big%20Buck%20Bunny"
)
TITLE = "Big Buck Bunny (Kademlia test)"


def play_magnet():
    item = xbmcgui.ListItem(label=TITLE, path=MAGNET_URI)
    item.setProperty("IsPlayable", "true")
    xbmcplugin.setResolvedUrl(PLUGIN_HANDLE, True, item)


def show_directory():
    item = xbmcgui.ListItem(label=TITLE)
    item.setInfo("video", {"title": TITLE, "mediatype": "movie"})
    item.setProperty("IsPlayable", "true")
    url = f"{PLUGIN_URL}?{urlencode({'action': 'play'})}"
    xbmcplugin.addDirectoryItem(PLUGIN_HANDLE, url, item, isFolder=False)
    xbmcplugin.endOfDirectory(PLUGIN_HANDLE)


params = parse_qs(sys.argv[2].lstrip("?"))
if params.get("action") == ["play"]:
    play_magnet()
else:
    show_directory()
