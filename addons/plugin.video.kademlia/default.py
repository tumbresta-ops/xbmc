import sys
from urllib.parse import parse_qs, parse_qsl, quote, urlsplit

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
    return parse_qs(query, keep_blank_values=True), query


def get_magnet(params, raw_query):
    magnet_values = params.get("magnet", [])
    if magnet_values and magnet_values[0]:
        return validate_magnet(magnet_values[0])

    action = params.get("action", [""])[0]
    if action.lower().startswith("magnet:"):
        if action.lower().startswith("magnet:magnet:?"):
            magnet_uri = action[len("magnet:") :]
        elif action.lower().startswith("magnet:?"):
            magnet_uri = action
        else:
            magnet_uri = f"magnet:?{action[len('magnet:'):]}"

        magnet_params = [
            (key, value)
            for key, value in parse_qsl(raw_query, keep_blank_values=True)
            if key not in ("action", "path", "magnet")
        ]
        if magnet_uri.lower().startswith("magnet:?"):
            existing_params = parse_qsl(urlsplit(magnet_uri).query, keep_blank_values=True)
            for param in magnet_params:
                if param not in existing_params:
                    existing_params.append(param)
            magnet_uri = f"magnet:?{urlencode(existing_params)}"

        return validate_magnet(magnet_uri)

    plugin_url = urlsplit(PLUGIN_URL)
    if plugin_url.path.rstrip("/").lower().endswith("/magnet:"):
        query = plugin_url.query or raw_query
        if query.lower().startswith("magnet:"):
            return validate_magnet(query)
        return validate_magnet(f"magnet:?{query.lstrip('?')}")

    raise ValueError("No magnet URI was provided.")


def validate_magnet(magnet_uri):
    magnet_uri = magnet_uri.strip()
    parsed = urlsplit(magnet_uri)
    if parsed.scheme.lower() != "magnet" or not parsed.query:
        raise ValueError("The supplied URL is not a valid magnet URI.")
    params = parse_qs(parsed.query, keep_blank_values=True)
    if not any(params.get("xt", [])):
        raise ValueError("Magnet URI is missing the xt parameter.")
    return magnet_uri


def add_torrent_folder(magnet_uri):
    magnet_uri = validate_magnet(magnet_uri)
    query = urlsplit(magnet_uri).query
    params = parse_qs(query, keep_blank_values=True)
    name = params.get("dn", ["Open torrent"])[0] or "Open torrent"
    torrent_url = f"magnet://{magnet_uri[len('magnet:'):]}"

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
        torrent_url = f"magnet://?{magnet_query}"
        item = xbmcgui.ListItem(label=name)
        item.setInfo("video", {"title": name, "mediatype": "video"})
        xbmcplugin.addDirectoryItem(
            PLUGIN_HANDLE,
            torrent_url,
            item,
            isFolder=True,
        )
    xbmcplugin.endOfDirectory(PLUGIN_HANDLE)


params, raw_query = parse_plugin_params()
plugin_path = urlsplit(PLUGIN_URL).path.rstrip("/")
has_magnet_action = params.get("action", [""])[0].lower().startswith("magnet:")
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
