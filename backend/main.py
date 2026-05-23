import os
import re
import json
import sys
import threading
import subprocess
import signal
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
import uvicorn
import webview
import ffmpeg

os.environ["QTWEBENGINE_DISABLE_SANDBOX"] = "1"
sys.argv.append("--no-sandbox")

app = FastAPI()
app.add_middleware(CORSMiddleware, allow_origins=["*"], allow_methods=["*"], allow_headers=["*"])

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
CONFIG_FILE = os.path.join(BASE_DIR, "config.json")

def load_config():
    if os.path.exists(CONFIG_FILE):
        with open(CONFIG_FILE, "r", encoding="utf-8") as f:
            return json.load(f)
    return {"video_dir": "", "last_played": "", "progress": {}, "tracks": {}, "watched": []}

def save_config(config_data):
    with open(CONFIG_FILE, "w", encoding="utf-8") as f:
        json.dump(config_data, f, ensure_ascii=False, indent=4)

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
    for root, _, files in os.walk(video_dir):
        for file in sorted(files):
            if file.endswith(extensions):
                full_path = os.path.join(root, file)
                rel_path = os.path.relpath(full_path, video_dir)
                meta = get_video_metadata(full_path)
                videos.append({
                    "id": rel_path, 
                    "title": meta["title"], 
                    "filename": file, 
                    "tracks": meta["tracks"],
                    "duration": meta["duration"]
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
        cmd = [
            "mpv",
            f"--aid={saved_track}",
            "--save-position-on-quit",
            full_path
        ]
        subprocess.run(cmd)

    threading.Thread(target=run_mpv).start()
    return {"status": "playing"}

@app.post("/api/toggle_watched")
def toggle_watched(data: dict):
    config = load_config()
    video_id = data.get("video_id")
    if "watched" not in config: config["watched"] = []
    
    if video_id in config["watched"]:
        config["watched"].remove(video_id)
    else:
        config["watched"].append(video_id)
        
    save_config(config)
    return {"status": "ok"}

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

class Api:
    def select_folder(self):
        result = window.create_file_dialog(webview.FOLDER_DIALOG)
        if result and len(result) > 0:
            config = load_config()
            config["video_dir"] = result[0]
            config["last_played"] = ""
            save_config(config)
            return result[0]
        return ""

def on_window_closed():
    print("🛑 Окно CINEBASE закрыто! Завершаем фоновые процессы...")
    
    pid_file = os.path.join(BASE_DIR, "..", "frontend.pid")
    if os.path.exists(pid_file):
        try:
            with open(pid_file, "r") as f:
                frontend_pid = int(f.read().strip())
            
            os.kill(frontend_pid, signal.SIGTERM)
            os.remove(pid_file)
            print("[Clean] Сервер фронтенда успешно остановлен.")
        except Exception as e:
            print(f"[Error] Не удалось остановить фронтенд: {e}")
            
    os.kill(os.getpid(), signal.SIGINT)

if __name__ == "__main__":
    threading.Thread(target=lambda: uvicorn.run(app, host="127.0.0.1", port=8000, log_level="warning"), daemon=True).start()
    
    window = webview.create_window(
        title="CINEBASE",
        url="http://localhost:5173",
        js_api=Api(),
        width=1200,
        height=800,
        resizable=True,
        fullscreen=False,
        min_size=(800, 600),
        confirm_close=False,
        background_color="#0B0B0F"
    )
    
    window.events.closed += on_window_closed
    
    webview.start(
        gui='qt',
        debug=False,
        http_server=True
    )