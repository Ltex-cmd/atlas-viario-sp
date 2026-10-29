// Pages serves the interface; the C query engine remains on the server.
export function apiURL(path: string) {
    const origin = (import.meta.env.VITE_ATLAS_API_URL || '').replace(/\/+$/, '');
    return origin + path;
}
