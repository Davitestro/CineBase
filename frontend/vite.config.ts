import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import fs from 'fs'
import path from 'path'

export default defineConfig({
  plugins: [
    react(),
    {
      name: 'save-pid',
      configureServer() {
        const pidFile = path.resolve(__dirname, '../frontend.pid')
        fs.writeFileSync(pidFile, process.pid.toString(), 'utf-8')
      }
    }
  ],
  server: {
    port: 5173,
    strictPort: true
  }
})