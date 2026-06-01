import os
import re
import json
import sys
import threading
import subprocess
import signal
import socket
import tempfile
import time
import uuid
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
import uvicorn
import ffmpeg

app = FastAPI()
app.add_middleware(CORSMiddleware, allow_origins=["*"], allow_methods=["*"], allow_headers=["*"])

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
CONFIG_FILE = os.path.join(BASE_DIR, "config.json")
CONFIG_LOCK = threading.Lock()

def load_config():
    if os.path.exists(CONFIG_FILE):
        with open(CONFIG_FILE, "r", encoding="utf-8") as f:
            config = json.load(f)
            config.setdefault("video_dir", "")
            config.setdefault("last_played", "")
            config.setdefault("progress", {})
            config.setdefault("tracks", {})
            config.setdefault("watched", [])
            return config
    return {"video_dir": "", "last_played": "", "progress": {}, "tracks": {}, "watched": []}

def save_config(config_data):
    with open(CONFIG_FILE, "w", encoding="utf-8") as f:
        json.dump(config_data, f, ensure_ascii=False, indent=4)

def save_video_progress(video_id, position):
    if not video_id or position is None:
        return

    with CONFIG_LOCK:
        config = load_config()
        config.setdefault("progress", {})
        config["progress"][video_id] = max(0.0, float(position))
        save_config(config)

def read_mpv_property(ipc_socket, property_name):
    try:
        command = json.dumps({"command": ["get_property", property_name]}) + "\n"
        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as client:
            client.settimeout(0.4)
            client.connect(ipc_socket)
            client.sendall(command.encode("utf-8"))
            response = client.recv(4096).decode("utf-8")
        data = json.loads(response.strip().splitlines()[-1])
        if data.get("error") == "success":
            return data.get("data")
    except Exception:
        return None
    return None

def track_mpv_progress(process, ipc_socket, video_id):
    last_position = None

    while process.poll() is None:
        position = read_mpv_property(ipc_socket, "time-pos")
        if isinstance(position, (int, float)):
            last_position = position
            save_video_progress(video_id, position)
        time.sleep(0.5)

    position = read_mpv_property(ipc_socket, "time-pos")
    if isinstance(position, (int, float)):
        last_position = position

    if last_position is not None:
        save_video_progress(video_id, last_position)

def get_video_metadata(file_path):
    try:
        probe = ffmpeg.probe(file_path)
        audio_streams = [stream for stream in probe['streams'] if stream['codec_type'] == 'audio']
        duration = float(probe.get('format', {}).get('duration', 0.0))
        
        title = probe.get('format', {}).get('tags', {}).get('title')
        if not title:
            clean_name = os.path.splitext(os.path.basename(file_path))[0]
            title = re.sub(r'\[.*?\]|\(.*?\)', '', clean_name).strip()

        tracks = []
        for i, stream in enumerate(audio_streams):
            lang = stream.get('tags', {}).get('language', f'Track {i+1}')
            title_track = stream.get('tags', {}).get('title', '')
            tracks.append({"index": i + 1, "label": f"[{lang}] {title_track}".strip()})

        return {"title": title, "tracks": tracks, "duration": duration}
    except Exception:
        clean_name = os.path.splitext(os.path.basename(file_path))[0]
        return {"title": clean_name, "tracks": [{"index": 1, "label": "По умолчанию"}], "duration": 0.0}

@app.get("/api/videos")
def list_videos():
    config = load_config()
    video_dir = config.get("video_dir", "")
    if not video_dir or not os.path.exists(video_dir):
        return {"videos": [], "video_dir": video_dir, "last_played": config.get("last_played", "")}
    
    videos = []
    extensions = ('.mp4', '.mkv', '.webm', '.avi')
    watched = set(config.get("watched", []))
    progress = config.get("progress", {})
    for root, _, files in os.walk(video_dir):
        for file in sorted(files):
            if file.lower().endswith(extensions):
                full_path = os.path.join(root, file)
                rel_path = os.path.relpath(full_path, video_dir)
                meta = get_video_metadata(full_path)
                videos.append({
                    "id": rel_path, 
                    "title": meta["title"], 
                    "filename": file, 
                    "tracks": meta["tracks"],
                    "duration": meta["duration"],
                    "watched": rel_path in watched,
                    "progress": progress.get(rel_path, 0)
                })
    return {"videos": videos, "video_dir": video_dir, "last_played": config.get("last_played", "")}

@app.post("/api/play")
def play_video(data: dict):
    config = load_config()
    video_id = data.get("video_id")
    video_dir = config.get("video_dir", "")
    full_path = os.path.join(video_dir, video_id)
    
    if not os.path.exists(full_path):
        raise HTTPException(status_code=404, detail="Файл не найден")

    saved_track = config.get("tracks", {}).get(video_id, 1)

    config["last_played"] = video_id
    save_config(config)

    def run_mpv():
        ipc_socket = os.path.join(tempfile.gettempdir(), f"cinebase-mpv-{uuid.uuid4().hex}.sock")
        cmd = [
            "mpv",
            f"--aid={saved_track}",
            "--save-position-on-quit",
            f"--input-ipc-server={ipc_socket}",
            full_path
        ]
        process = subprocess.Popen(cmd)
        try:
            track_mpv_progress(process, ipc_socket, video_id)
            process.wait()
        finally:
            if os.path.exists(ipc_socket):
                os.remove(ipc_socket)

    threading.Thread(target=run_mpv, daemon=True).start()
    return {"status": "playing"}

@app.post("/api/toggle_watched")
def toggle_watched(data: dict):
    config = load_config()
    video_id = data.get("video_id")
    if not video_id:
        raise HTTPException(status_code=400, detail="video_id is required")
    if "watched" not in config: config["watched"] = []
    
    if video_id in config["watched"]:
        config["watched"].remove(video_id)
        watched = False
    else:
        config["watched"].append(video_id)
        watched = True
        
    save_config(config)
    return {"status": "ok", "video_id": video_id, "watched": watched, "watched_list": config["watched"]}

@app.post("/api/set_track")
def set_track(data: dict):
    config = load_config()
    video_id = data.get("video_id")
    track_index = data.get("track_index", 1)
    if "tracks" not in config: config["tracks"] = {}
    config["tracks"][video_id] = track_index
    save_config(config)
    return {"status": "ok"}

@app.get("/api/progress")
def get_all_progress():
    config = load_config()
    return {
        "progress": config.get("progress", {}),
        "tracks": config.get("tracks", {}),
        "watched": config.get("watched", [])
    }

@app.post("/api/set_video_dir")
def set_video_dir(data: dict):
    """Set the video directory (called by Qt frontend)"""
    config = load_config()
    video_dir = data.get("video_dir", "")
    
    if video_dir and os.path.isdir(video_dir):
        config["video_dir"] = video_dir
        config["last_played"] = ""
        save_config(config)
        return {"status": "ok", "video_dir": video_dir}
    else:
        raise HTTPException(status_code=400, detail="Invalid directory")

@app.get("/api/status")
def status():
    """Check backend status"""
    return {"status": "running", "message": "CINEBASE Backend API"}

@app.post("/api/shutdown")
def shutdown():
    """Stop the backend when the desktop frontend exits."""
    def stop_server():
        os.kill(os.getpid(), signal.SIGTERM)

    threading.Timer(0.2, stop_server).start()
    return {"status": "shutting_down"}

if __name__ == "__main__":
    print("╔════════════════════════════════════╗")
    print("║     CINEBASE Backend API Server    ║")
    print("║      (Qt C++ Frontend Edition)     ║")
    print("╚════════════════════════════════════╝")
    print("")
    print("🔵 Starting FastAPI Server...")
    print("📡 Listening on: http://127.0.0.1:8000")
    print("📚 API Docs: http://127.0.0.1:8000/docs")
    print("")
    print("Press Ctrl+C to stop the server")
    print("")
    
    uvicorn.run(app, host="127.0.0.1", port=8000, log_level="info")
