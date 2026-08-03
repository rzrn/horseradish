from datetime import datetime
from typing import Optional, Dict, Any

from packaging import version

from pyspades.logger import getLogger
from horseradish.version import __version__


log = getLogger()


async def fetch_latest_release() -> Dict[str, Any]:
    import aiohttp

    endpoint = "https://api.github.com/repos/rzrn/horseradish/releases/latest"
    async with aiohttp.ClientSession() as session:
        async with session.get(endpoint) as response:
            return await response.json()


def format_release(release: Dict[str, Any]) -> str:
    latest_version = release["tag_name"]
    date = datetime.strptime(release["published_at"], "%Y-%m-%dT%H:%M:%SZ")
    formated = date.strftime("%b %d %Y")
    # git.io url points towards /latest release page
    return "New release available: {} ({}): https://git.io/fjIDk".format(latest_version, formated)


async def check_for_releases() -> Optional[Dict[str, Any]]:
    """Checks for new release and returns it if new release is found."""

    log.debug("Checking latest version")
    try:
        release = await fetch_latest_release()
    except IOError as e:
        log.warning("Could not fetch latest version: {err}", err = e)
        return
    except ImportError:
        log.warning("Could not fetch latest version: `aiohttp` is not installed")
        return

    if latest_version := release.get("tag_name"):
        if version.parse(latest_version) > version.parse(__version__):
            return release

    return None
