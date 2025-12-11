#!/bin/bash
# Custom Fortress 2 - Linux Dedicated Server Launcher
# 
# REQUIREMENTS:
# 1. Install Custom Fortress 2 Dedicated Server via SteamCMD:
#    steamcmd +force_install_dir /path/to/cf2ds +login anonymous +app_update 4231320 validate +quit
#
# 2. Get a Game Server Login Token (GSLT) from:
#    https://steamcommunity.com/dev/managegameservers
#    Use App ID 440 (TF2) when creating the token
#
# USAGE:
#    ./start_server.sh [additional srcds parameters]
#
# EXAMPLES:
#    ./start_server.sh +map ctf_2fort +maxplayers 24
#    ./start_server.sh +sv_setsteamaccount YOUR_GSLT_TOKEN_HERE +map ctf_2fort +maxplayers 24 +sv_lan 0
#    ./start_server.sh +sv_use_steam_networking 1 +map ctf_2fort +maxplayers 24

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Check if srcds_run_64 or srcds_linux64 exists
if [ ! -f "$SCRIPT_DIR/srcds_run_64" ] && [ ! -f "$SCRIPT_DIR/srcds_linux64" ]; then
    echo "ERROR: srcds_run_64 or srcds_linux64 not found!"
    echo ""
    echo "You need to install the Custom Fortress 2 Dedicated Server first:"
    echo "  steamcmd +force_install_dir \"$SCRIPT_DIR\" +login anonymous +app_update 4231320 validate +quit"
    echo ""
    exit 1
fi

# Check if customfortress folder exists
if [ ! -d "$SCRIPT_DIR/customfortress" ]; then
    echo "ERROR: customfortress folder not found!"
    echo "Make sure the Custom Fortress 2 dedicated server is properly installed."
    exit 1
fi

# Use server-specific gameinfo.txt (handles paths for dedicated server)
if [ -f "$SCRIPT_DIR/customfortress/gameinfo_server.txt" ]; then
    cp "$SCRIPT_DIR/customfortress/gameinfo_server.txt" "$SCRIPT_DIR/customfortress/gameinfo.txt"
    echo "Using server gameinfo configuration."
fi

# Create server binary symlinks following Source SDK 2013 dedicated server requirements
echo "Creating necessary symlinks..."

# Main engine symlinks in bin/linux64/
BINDIR="$SCRIPT_DIR/bin/linux64"
if [ -d "$BINDIR" ]; then
    cd "$BINDIR"
    [ -f "datacache_srv.so" ] && ln -sf datacache_srv.so datacache.so
    [ -f "dedicated_srv.so" ] && ln -sf dedicated_srv.so dedicated.so
    [ -f "engine_srv.so" ] && ln -sf engine_srv.so engine.so
    [ -f "libtier0_srv.so" ] && ln -sf libtier0_srv.so libtier0.so
    [ -f "libvstdlib_srv.so" ] && ln -sf libvstdlib_srv.so libvstdlib.so
    [ -f "materialsystem_srv.so" ] && ln -sf materialsystem_srv.so materialsystem.so
    [ -f "replay_srv.so" ] && ln -sf replay_srv.so replay.so
    [ -f "scenefilecache_srv.so" ] && ln -sf scenefilecache_srv.so scenefilecache.so
    [ -f "shaderapiempty_srv.so" ] && ln -sf shaderapiempty_srv.so shaderapiempty.so
    [ -f "soundemittersystem_srv.so" ] && ln -sf soundemittersystem_srv.so soundemittersystem.so
    [ -f "studiorender_srv.so" ] && ln -sf studiorender_srv.so studiorender.so
    [ -f "vphysics_srv.so" ] && ln -sf vphysics_srv.so vphysics.so
    [ -f "vscript_srv.so" ] && ln -sf vscript_srv.so vscript.so
    cd "$SCRIPT_DIR"
fi

# Mod binary symlinks in customfortress/bin/linux64/
MODBIN="$SCRIPT_DIR/customfortress/bin/linux64"
if [ -d "$MODBIN" ]; then
    cd "$MODBIN"
    [ -f "server.so" ] && ln -sf server.so server_srv.so
    [ -f "libtier0.so" ] && ln -sf libtier0.so libtier0_srv.so
    [ -f "libvstdlib.so" ] && ln -sf libvstdlib.so libvstdlib_srv.so
    cd "$SCRIPT_DIR"
fi

echo "Symlinks created."

# Ensure SteamCMD SDK symlink exists
if [ ! -d "$HOME/.steam/sdk64" ] && [ -d "$HOME/.steam/steam/steamcmd/linux64" ]; then
    echo "Creating Steam SDK symlink..."
    ln -s "$HOME/.steam/steam/steamcmd/linux64" "$HOME/.steam/sdk64"
fi

# Set library path for Source engine shared libraries
export LD_LIBRARY_PATH="$SCRIPT_DIR/bin/linux64:$SCRIPT_DIR/customfortress/bin/linux64:$LD_LIBRARY_PATH"
export SteamEnv=1

# Launch the server
cd "$SCRIPT_DIR"

# Use srcds_run_64 if available (wrapper script), otherwise use srcds_linux64 directly
if [ -f "./srcds_run_64" ]; then
    echo "Starting Custom Fortress 2 Dedicated Server (using srcds_run_64)..."
    # Note: -insecure disables VAC. Remove it for production servers that need VAC protection.
    ./srcds_run_64 -game customfortress -console +sv_lan 0 "$@"
else
    echo "Starting Custom Fortress 2 Dedicated Server (using srcds_linux64)..."
    # Note: -insecure disables VAC. Remove it for production servers that need VAC protection.
    ./srcds_linux64 -game customfortress -console +sv_lan 0 "$@"
fi