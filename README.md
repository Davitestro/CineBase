# Movie for

Small full-stack movie app (frontend + backend).

## Layout
- `backend/` - Python backend (main.py, config.json)
- `frontend/` - TypeScript + Vite frontend

## Requirements
- Python 3.10+ (venv recommended)
- Node 16+ and npm or yarn
- System packages: `ffmpeg` and `mpv` (required by the backend to probe media and play videos)

## Backend (dev)
1. Create and activate virtualenv:

```bash
python -m venv .venv
source .venv/bin/activate
```

2. Install Python dependencies and run the backend:

```bash
pip install -r requirements.txt
python backend/main.py
```

Note: `ffmpeg` and `mpv` are external binaries and should be installed via your OS package manager (e.g. `sudo apt install ffmpeg mpv`).

## Frontend (dev)
1. From `frontend/` install and run:

```bash
cd frontend
npm install
npm run dev
```

2. Open browser at the address printed by Vite (usually http://localhost:5173).

## Notes

- Update `backend/config.json` to set backend settings before running.

## Want me to?
- Add `requirements.txt` and a minimal start script.
