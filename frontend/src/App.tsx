import React, { useState, useEffect } from 'react';

interface AudioTrack {
  index: number;
  label: string;
}

interface VideoFile {
  id: string;
  title: string;
  filename: string;
  tracks: AudioTrack[];
  duration: number;
}

const API_URL = 'http://localhost:8000/api';

export default function App() {
  const [videos, setVideos] = useState<VideoFile[]>([]);
  const [currentDir, setCurrentDir] = useState<string>('');
  const [selectedVideo, setSelectedVideo] = useState<VideoFile | null>(null);
  const [searchQuery, setSearchQuery] = useState<string>('');
  const [history, setHistory] = useState<{ [key: string]: number }>({});
  const [savedTracks, setSavedTracks] = useState<{ [key: string]: number }>({});
  const [serverWatched, setServerWatched] = useState<string[]>([]);
  const [hoveredVideo, setHoveredVideo] = useState<string | null>(null);
  const [windowSize, setWindowSize] = useState({
    width: typeof window !== 'undefined' ? window.innerWidth : 1280,
    height: typeof window !== 'undefined' ? window.innerHeight : 800,
  });

  const loadLibrary = () => {
    fetch(`${API_URL}/videos`)
      .then(res => res.json())
      .then(data => {
        setVideos(data.videos || []);
        setCurrentDir(data.video_dir || '');
        
        fetch(`${API_URL}/progress`)
          .then(r => r.json())
          .then(progData => {
            setHistory(progData.progress || {});
            setSavedTracks(progData.tracks || {});
            setServerWatched(progData.watched || []);
            
            if (data.last_played && data.videos) {
              const last = data.videos.find((v: VideoFile) => v.id === data.last_played);
              if (last) setSelectedVideo(last);
            }
          });
      })
      .catch(err => console.error("Sync error:", err));
  };

  useEffect(() => {
    loadLibrary();
    
    const handleResize = () => {
      setWindowSize({
        width: window.innerWidth,
        height: window.innerHeight,
      });
    };
    
    window.addEventListener('resize', handleResize);
    
    const interval = setInterval(() => {
      fetch(`${API_URL}/progress`)
        .then(r => r.json())
        .then(d => {
          setHistory(d.progress || {});
          setServerWatched(d.watched || []);
        });
    }, 2000);
    
    return () => {
      clearInterval(interval);
      window.removeEventListener('resize', handleResize);
    };
  }, []);

  const handleSelectFolder = async () => {
    if ((window as any).pywebview && (window as any).pywebview.api) {
      const folder = await (window as any).pywebview.api.select_folder();
      if (folder) {
        setSelectedVideo(null);
        setTimeout(() => loadLibrary(), 500);
      }
    }
  };

  const handlePlay = (video_id: string) => {
    fetch(`${API_URL}/play`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ video_id })
    });
  };

  const handleTrackChange = (video_id: string, trackIndex: number) => {
    fetch(`${API_URL}/set_track`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ video_id, track_index: trackIndex })
    }).then(() => loadLibrary());
  };

  const toggleWatched = (video_id: string) => {
    fetch(`${API_URL}/toggle_watched`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ video_id })
    }).then(() => loadLibrary());
  };

  const formatTime = (seconds: number) => {
    if (!seconds || seconds === 0) return '00:00';
    const h = Math.floor(seconds / 3600);
    const m = Math.floor((seconds % 3600) / 60);
    const s = Math.floor(seconds % 60);
    if (h > 0) {
      return `${h}:${m.toString().padStart(2, '0')}:${s.toString().padStart(2, '0')}`;
    }
    return `${m}:${s.toString().padStart(2, '0')}`;
  };

  const getProgressPercent = (videoId: string) => {
    const time = history[videoId] || 0;
    const duration = videos.find(v => v.id === videoId)?.duration || 1;
    if (duration === 0) return 0;
    return (time / duration) * 100;
  };

  const filteredVideos = videos.filter(v => 
    v.title.toLowerCase().includes(searchQuery.toLowerCase()) || 
    v.filename.toLowerCase().includes(searchQuery.toLowerCase())
  );

  return (
    <div style={styles.app}>
      <aside style={styles.sidebar}>
        <div style={styles.header}>
          <div style={styles.logoIcon}>◆</div>
          <span style={styles.logoText}>CINEBASE</span>
        </div>

        <button onClick={handleSelectFolder} style={styles.folderBtn}>
          <span>📂</span>
          <span>Media Root</span>
        </button>

        {currentDir && (
          <div style={styles.dirInfo}>
            <span style={styles.dirLabel}>CURRENT</span>
            <span style={styles.dirPath}>{currentDir.split('/').pop() || currentDir.split('\\').pop()}</span>
          </div>
        )}

        <div style={styles.searchContainer}>
          <input 
            type="text"
            placeholder="Search..."
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
            style={styles.searchInput}
          />
        </div>

        <div style={styles.libraryHeader}>
          <span>LIBRARY</span>
          <span style={styles.count}>{filteredVideos.length}</span>
        </div>

        <div style={styles.videoList}>
          {filteredVideos.map((video) => {
            const isSelected = selectedVideo?.id === video.id;
            const isWatched = serverWatched.includes(video.id);
            const progress = getProgressPercent(video.id);
            const isHovered = hoveredVideo === video.id;
            
            return (
              <div
                key={video.id}
                onClick={() => setSelectedVideo(video)}
                onMouseEnter={() => setHoveredVideo(video.id)}
                onMouseLeave={() => setHoveredVideo(null)}
                style={{
                  ...styles.videoItem,
                  ...(isSelected ? styles.videoItemSelected : {}),
                  ...(isWatched ? styles.videoItemWatched : {})
                }}
              >
                <div style={styles.videoContent}>
                  <div style={styles.videoInfo}>
                    <div style={styles.videoTitle}>{video.title}</div>
                    {!isWatched && history[video.id] > 3 && (
                      <span style={styles.resumeBadge}>{formatTime(history[video.id])}</span>
                    )}
                  </div>
                  <div style={styles.videoMeta}>{video.filename}</div>
                  {!isWatched && progress > 0 && progress < 95 && (
                    <div style={styles.progressTrack}>
                      <div style={{ ...styles.progressFill, width: `${progress}%` }} />
                    </div>
                  )}
                </div>
                
                <button
                  onClick={(e) => { e.stopPropagation(); toggleWatched(video.id); }}
                  style={{
                    ...styles.watchedBtn,
                    ...(isWatched ? styles.watchedBtnActive : {}),
                    ...(isHovered && !isWatched ? styles.watchedBtnHover : {})
                  }}
                >
                  {isWatched ? '✓' : '○'}
                </button>

                {isSelected && <div style={styles.selectedIndicator} />}
              </div>
            );
          })}
        </div>
      </aside>

      <main style={styles.main}>
        {selectedVideo ? (
          <div style={styles.heroContainer}>
            <div style={styles.heroGlow} />
            
            <div style={styles.heroContent}>
              <div style={styles.heroLeft}>
                <div style={styles.badgeContainer}>
                  {serverWatched.includes(selectedVideo.id) ? (
                    <span style={styles.badgeWatched}>
                      <span style={styles.badgeDot} /> COMPLETED
                    </span>
                  ) : history[selectedVideo.id] > 3 ? (
                    <span style={styles.badgeResume}>
                      <span style={styles.badgeDot} /> RESUME AT {formatTime(history[selectedVideo.id])}
                    </span>
                  ) : (
                    <span style={styles.badgeNew}>
                      <span style={styles.badgeDot} /> READY
                    </span>
                  )}
                </div>

                <h1 style={styles.heroTitle}>{selectedVideo.title}</h1>
                <p style={styles.heroFilename}>{selectedVideo.filename}</p>

                <div style={styles.trackContainer}>
                  <label style={styles.trackLabel}>AUDIO TRACK</label>
                  <div style={styles.selectWrapper}>
                    <select 
                      style={styles.trackSelect}
                      value={savedTracks[selectedVideo.id] || 1}
                      onChange={(e) => handleTrackChange(selectedVideo.id, parseInt(e.target.value))}
                    >
                      {selectedVideo.tracks.map(track => (
                        <option key={track.index} value={track.index}>
                          {track.label}
                        </option>
                      ))}
                    </select>
                    <div style={styles.selectChevron}>⌄</div>
                  </div>
                </div>

                <button 
                  onClick={() => handlePlay(selectedVideo.id)} 
                  style={styles.playBtn}
                  onMouseEnter={(e) => {
                    e.currentTarget.style.transform = 'translateY(-2px)';
                    e.currentTarget.style.boxShadow = '0 8px 20px rgba(0,216,160,0.3)';
                  }}
                  onMouseLeave={(e) => {
                    e.currentTarget.style.transform = 'translateY(0)';
                    e.currentTarget.style.boxShadow = 'none';
                  }}
                >
                  <span style={styles.playIcon}>▶</span>
                  PLAY
                </button>
              </div>

              <div style={styles.heroRight}>
                <div style={styles.artworkFrame}>
                  <div style={styles.artworkInner}>
                    <div style={styles.artworkEmoji}>🎬</div>
                    <div style={styles.artworkPulse} />
                  </div>
                </div>
                
                <div style={styles.statsGrid}>
                  <div style={styles.statItem}>
                    <span style={styles.statValue}>{selectedVideo.tracks.length}</span>
                    <span style={styles.statLabel}>audio tracks</span>
                  </div>
                  <div style={styles.statItem}>
                    <span style={styles.statValue}>
                      {selectedVideo.duration ? formatTime(selectedVideo.duration) : '--:--'}
                    </span>
                    <span style={styles.statLabel}>duration</span>
                  </div>
                </div>
              </div>
            </div>

            <div style={styles.quickActions}>
              <button 
                onClick={() => handlePlay(selectedVideo.id)} 
                style={styles.quickPlay}
                onMouseEnter={(e) => (e.currentTarget.style.transform = 'translateY(-2px)')}
                onMouseLeave={(e) => (e.currentTarget.style.transform = 'translateY(0)')}
              >
                ▶ Play
              </button>
              <button 
                onClick={() => toggleWatched(selectedVideo.id)}
                style={styles.quickMark}
                onMouseEnter={(e) => (e.currentTarget.style.transform = 'translateY(-2px)')}
                onMouseLeave={(e) => (e.currentTarget.style.transform = 'translateY(0)')}
              >
                {serverWatched.includes(selectedVideo.id) ? '✓ Watched' : '○ Mark watched'}
              </button>
            </div>
          </div>
        ) : (
          <div style={styles.emptyState}>
            <div style={styles.emptyIcon}>◧</div>
            <p style={styles.emptyText}>select video to begin</p>
            <div style={styles.emptyHint}>
              <span>📂</span> add media folder to start
            </div>
          </div>
        )}
      </main>
    </div>
  );
}

const styles: { [key: string]: React.CSSProperties } = {
  app: { display: 'flex', width: '100%', height: '100%', background: '#0B0B0F', color: '#EAEAEF', fontFamily: `'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif`, overflow: 'hidden', position: 'fixed', top: 0, left: 0, right: 0, bottom: 0 },
  sidebar: { width: '340px', background: '#0F0F14', borderRight: '1px solid rgba(255,255,255,0.05)', display: 'flex', flexDirection: 'column', overflow: 'hidden', flexShrink: 0 },
  header: { display: 'flex', alignItems: 'center', gap: '10px', padding: '24px 20px', borderBottom: '1px solid rgba(255,255,255,0.05)', marginBottom: '16px' },
  logoIcon: { fontSize: '20px', color: '#00D8A0' },
  logoText: { fontSize: '16px', fontWeight: 600, letterSpacing: '1px', background: 'linear-gradient(135deg, #fff 0%, #888 100%)', WebkitBackgroundClip: 'text', WebkitTextFillColor: 'transparent', backgroundClip: 'text' },
  folderBtn: { display: 'flex', alignItems: 'center', gap: '12px', margin: '0 16px 16px 16px', padding: '10px 16px', background: 'rgba(0,216,160,0.08)', border: '1px solid rgba(0,216,160,0.2)', borderRadius: '8px', color: '#00D8A0', fontSize: '13px', fontWeight: 500, cursor: 'pointer', transition: 'all 0.2s' },
  dirInfo: { margin: '0 16px 20px 16px', padding: '10px 12px', background: 'rgba(255,255,255,0.02)', borderRadius: '6px' },
  dirLabel: { display: 'block', color: '#555', fontSize: '9px', textTransform: 'uppercase', letterSpacing: '0.5px', marginBottom: '4px' },
  dirPath: { color: '#888', fontFamily: 'monospace', fontSize: '11px', wordBreak: 'break-all' },
  searchContainer: { margin: '0 16px 20px 16px' },
  searchInput: { width: '100%', padding: '10px 14px', background: '#0A0A0E', border: '1px solid rgba(255,255,255,0.05)', borderRadius: '8px', color: '#EAEAEF', fontSize: '13px', outline: 'none', transition: 'all 0.2s' },
  libraryHeader: { display: 'flex', justifycontent: 'space-between', justifyContent: 'space-between', padding: '0 20px', marginBottom: '12px', fontSize: '11px', fontWeight: 600, letterSpacing: '0.5px', color: '#555', textTransform: 'uppercase' },
  count: { color: '#00D8A0' },
  videoList: { flex: 1, overflowY: 'auto', paddingBottom: '20px' },
  videoItem: { position: 'relative', display: 'flex', alignItems: 'center', justifycontent: 'space-between', justifyContent: 'space-between', cursor: 'pointer', transition: 'all 0.2s', marginBottom: '2px', paddingRight: '12px' },
  videoContent: { flex: 1, padding: '12px 0 12px 20px' },
  videoItemSelected: { background: 'rgba(0,216,160,0.04)' },
  videoItemWatched: { opacity: 0.4 },
  videoInfo: { display: 'flex', justifycontent: 'space-between', justifyContent: 'space-between', alignItems: 'baseline', marginBottom: '4px', gap: '8px' },
  videoTitle: { fontSize: '13px', fontWeight: 500, overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap', flex: 1 },
  resumeBadge: { fontSize: '10px', color: '#00D8A0', fontFamily: 'monospace', background: 'rgba(0,216,160,0.1)', padding: '2px 6px', borderRadius: '4px', flexShrink: 0 },
  videoMeta: { fontSize: '10px', color: '#555', marginBottom: '6px', overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap' },
  progressTrack: { height: '2px', background: 'rgba(255,255,255,0.05)', borderRadius: '1px', overflow: 'hidden', marginTop: '6px' },
  progressFill: { height: '100%', background: '#00D8A0', borderRadius: '1px', transition: 'width 0.3s ease' },
  watchedBtn: { width: '28px', height: '28px', background: 'transparent', border: 'none', color: '#444', fontSize: '14px', cursor: 'pointer', borderRadius: '6px', transition: 'all 0.2s', display: 'flex', alignItems: 'center', justifycontent: 'center', justifyContent: 'center', flexShrink: 0 },
  watchedBtnHover: { background: 'rgba(255,255,255,0.05)', color: '#888' },
  watchedBtnActive: { color: '#00D8A0' },
  selectedIndicator: { position: 'absolute', left: 0, top: '20%', height: '60%', width: '3px', background: '#00D8A0', borderRadius: '0 2px 2px 0' },
  main: { flex: 1, position: 'relative', overflow: 'auto' },
  heroContainer: { minHeight: '100%', position: 'relative', display: 'flex', flexDirection: 'column', justifycontent: 'center', justifyContent: 'center', padding: '60px' },
  heroGlow: { position: 'absolute', top: '50%', left: '50%', transform: 'translate(-50%, -50%)', width: '70%', height: '70%', background: 'radial-gradient(circle, rgba(0,216,160,0.08) 0%, rgba(0,216,160,0) 70%)', pointerEvents: 'none' },
  heroContent: { position: 'relative', zIndex: 2, display: 'flex', justifycontent: 'space-between', justifyContent: 'space-between', alignItems: 'center', gap: '60px', flexWrap: 'wrap' },
  heroLeft: { flex: 1, minWidth: '280px' },
  badgeContainer: { marginBottom: '20px' },
  badgeWatched: { display: 'inline-flex', alignItems: 'center', gap: '8px', padding: '6px 12px', background: 'rgba(255,255,255,0.03)', borderRadius: '20px', fontSize: '11px', fontWeight: 500, letterSpacing: '0.5px', color: '#888' },
  badgeResume: { display: 'inline-flex', alignItems: 'center', gap: '8px', padding: '6px 12px', background: 'rgba(0,216,160,0.1)', borderRadius: '20px', fontSize: '11px', fontWeight: 500, letterSpacing: '0.5px', color: '#00D8A0' },
  badgeNew: { display: 'inline-flex', alignItems: 'center', gap: '8px', padding: '6px 12px', background: 'rgba(0,216,160,0.05)', borderRadius: '20px', fontSize: '11px', fontWeight: 500, letterSpacing: '0.5px', color: '#00D8A0' },
  badgeDot: { width: '6px', height: '6px', background: '#00D8A0', borderRadius: '50%', display: 'inline-block' },
  heroTitle: { fontSize: '42px', fontWeight: 600, letterSpacing: '-1px', margin: '0 0 12px 0', background: 'linear-gradient(135deg, #fff 0%, #aaa 100%)', WebkitBackgroundClip: 'text', WebkitTextFillColor: 'transparent', backgroundClip: 'text' },
  heroFilename: { fontSize: '13px', color: '#555', fontFamily: 'monospace', margin: '0 0 32px 0' },
  trackContainer: { marginBottom: '32px' },
  trackLabel: { display: 'block', fontSize: '10px', fontWeight: 600, letterSpacing: '1px', color: '#555', marginBottom: '8px', textTransform: 'uppercase' },
  selectWrapper: { position: 'relative', maxWidth: '280px' },
  trackSelect: { width: '100%', padding: '12px 36px 12px 14px', background: '#0F0F14', border: '1px solid rgba(255,255,255,0.08)', borderRadius: '8px', color: '#EAEAEF', fontSize: '13px', cursor: 'pointer', appearance: 'none', fontFamily: 'monospace' },
  selectChevron: { position: 'absolute', right: '14px', top: '50%', transform: 'translateY(-50%)', color: '#555', fontSize: '12px', pointerEvents: 'none' },
  playBtn: { display: 'inline-flex', alignItems: 'center', gap: '12px', padding: '14px 32px', background: '#00D8A0', border: 'none', borderRadius: '40px', color: '#0B0B0F', fontSize: '14px', fontWeight: 600, letterSpacing: '1px', cursor: 'pointer', transition: 'transform 0.2s, box-shadow 0.2s' },
  playIcon: { fontSize: '12px' },
  heroRight: { display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '24px' },
  artworkFrame: { width: '200px', height: '200px', borderRadius: '20px', background: 'linear-gradient(135deg, #1A1A24 0%, #0F0F14 100%)', border: '1px solid rgba(255,255,255,0.05)', display: 'flex', alignItems: 'center', justifycontent: 'center', justifyContent: 'center', position: 'relative' },
  artworkInner: { position: 'relative' },
  artworkEmoji: { fontSize: '64px', position: 'relative', zIndex: 2 },
  artworkPulse: { position: 'absolute', top: '50%', left: '50%', width: '120%', height: '120%', transform: 'translate(-50%, -50%)', borderRadius: '50%', background: 'radial-gradient(circle, rgba(0,216,160,0.2) 0%, rgba(0,216,160,0) 70%)', pointerEvents: 'none' },
  statsGrid: { display: 'flex', gap: '32px' },
  statItem: { textAlign: 'center' },
  statValue: { display: 'block', fontSize: '20px', fontWeight: 600, color: '#fff', marginBottom: '4px' },
  statLabel: { fontSize: '9px', color: '#555', textTransform: 'uppercase', letterSpacing: '0.5px' },
  quickActions: { position: 'relative', zIndex: 2, display: 'flex', gap: '12px', marginTop: '60px', paddingTop: '40px', borderTop: '1px solid rgba(255,255,255,0.05)' },
  quickPlay: { padding: '10px 24px', background: 'transparent', border: '1px solid rgba(0,216,160,0.3)', borderRadius: '30px', color: '#00D8A0', fontSize: '12px', fontWeight: 500, cursor: 'pointer', transition: 'transform 0.2s' },
  quickMark: { padding: '10px 24px', background: 'transparent', border: '1px solid rgba(255,255,255,0.08)', borderRadius: '30px', color: '#888', fontSize: '12px', fontWeight: 500, cursor: 'pointer', transition: 'transform 0.2s' },
  emptyState: { height: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center', justifycontent: 'center', justifyContent: 'center', gap: '16px' },
  emptyIcon: { fontSize: '80px', color: '#1A1A24', fontWeight: 300 },
  emptyText: { fontSize: '14px', color: '#444', letterSpacing: '0.5px' },
  emptyHint: { fontSize: '12px', color: '#333', display: 'flex', alignItems: 'center', gap: '6px' }
};