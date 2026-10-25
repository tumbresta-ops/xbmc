import sys
from urllib.parse import parse_qs, parse_qsl, quote, urlencode, urlsplit

import xbmcgui
import xbmcplugin


PLUGIN_URL = sys.argv[0]
PLUGIN_HANDLE = int(sys.argv[1])
TEST_TORRENTS = (
    (
        "Big Buck Bunny",
        "dd8255ecdc7ca55fb0bbf81323d87062db1f6d1c",
    ),
    (
        "Sintel",
        "08ada5a7a6183aae1e09d831df6748d566095a10",
    ),
    (
        "Tears of Steel",
        "209c8226b299b308beaf2b9cd3fb49212dbd13ec",
    ),
)


def parse_plugin_params():
    query = sys.argv[2].lstrip("?") if len(sys.argv) > 2 else ""
    return parse_qs(query), query


def get_magnet(params, raw_query):
    if params.get("magnet"):
        return params["magnet"][0]

    action = params.get("action", [""])[0]
    if action.startswith("magnet:"):
        magnet_uri = action[len("magnet:") :]
        magnet_params = [
            (key, value)
            for key, value in parse_qsl(raw_query, keep_blank_values=True)
            if key not in ("action", "path", "magnet")
        ]
        if magnet_uri.startswith("magnet:?"):
            existing_params = parse_qsl(urlsplit(magnet_uri).query, keep_blank_values=True)
            remaining_params = list(magnet_params)
            for param in existing_params:
                if param in remaining_params:
                    remaining_params.remove(param)
            if remaining_params:
                magnet_uri += f"&{urlencode(remaining_params)}"
            return magnet_uri
        if magnet_uri.startswith("magnet:"):
            magnet_uri += "?"
        elif magnet_uri.startswith("?"):
            magnet_uri = f"magnet:{magnet_uri}"
        elif magnet_params:
            magnet_uri = "magnet:?"
        else:
            raise ValueError("The action parameter must contain a magnet URI.")

        if magnet_params:
            magnet_uri += urlencode(magnet_params)
        return magnet_uri

    plugin_url = urlsplit(PLUGIN_URL)
    if plugin_url.path.rstrip("/").endswith("/magnet:"):
        query = plugin_url.query or raw_query
        if query.startswith("magnet:"):
            return query
        return f"magnet:?{query.lstrip('?')}"

    raise ValueError("No magnet URI was provided.")


def get_magnet_query(magnet_uri):
    parsed = urlsplit(magnet_uri)
    if parsed.scheme != "magnet" or not parsed.query:
        raise ValueError("The supplied URL is not a valid magnet URI.")
    return parsed.query


def add_torrent_folder(magnet_uri):
    query = get_magnet_query(magnet_uri)
    torrent_url = f"magnet://?{query}"
    name = parse_qs(query).get("dn", ["Open torrent"])[0]

    item = xbmcgui.ListItem(label=name)
    item.setInfo("video", {"title": name, "mediatype": "video"})
    xbmcplugin.addDirectoryItem(
        PLUGIN_HANDLE,
        torrent_url,
        item,
        isFolder=True,
    )
    xbmcplugin.endOfDirectory(PLUGIN_HANDLE)


def show_test_torrents():
    for name, info_hash in TEST_TORRENTS:
        magnet_query = f"xt=urn:btih:{info_hash}&dn={quote(name)}"
        item = xbmcgui.ListItem(label=name)
        item.setInfo("video", {"title": name, "mediatype": "video"})
        xbmcplugin.addDirectoryItem(
            PLUGIN_HANDLE,
            f"magnet://?{magnet_query}",
            item,
            isFolder=True,
        )
    xbmcplugin.endOfDirectory(PLUGIN_HANDLE)


params, raw_query = parse_plugin_params()
plugin_path = urlsplit(PLUGIN_URL).path.rstrip("/")
has_magnet_action = params.get("action", [""])[0].startswith("magnet:")
if (
    plugin_path in ("", "/")
    and not params.get("magnet")
    and not has_magnet_action
    and not raw_query
):
    show_test_torrents()
else:
    try:
        magnet_uri = get_magnet(params, raw_query)
        add_torrent_folder(magnet_uri)
    except ValueError as error:
        xbmcgui.Dialog().notification("Kademlia", str(error), xbmcgui.NOTIFICATION_ERROR)
        xbmcplugin.endOfDirectory(PLUGIN_HANDLE, succeeded=False)
