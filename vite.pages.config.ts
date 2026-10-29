import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import { fileURLToPath } from 'node:url';

export default defineConfig({
    root: fileURLToPath(new URL('./pages', import.meta.url)),
    envDir: fileURLToPath(new URL('.', import.meta.url)),
    cacheDir: '../.pages-cache',
    base: '/atlas-viario-sp/',
    publicDir: fileURLToPath(new URL('./pages-public', import.meta.url)),
    plugins: [react()],
    build: { outDir: '../dist-pages', emptyOutDir: true },
    server: { host: '127.0.0.1', port: 5174, proxy: { '/api/v1': 'http://127.0.0.1:5173' } },
});
