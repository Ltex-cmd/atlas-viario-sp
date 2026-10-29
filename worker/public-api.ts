const pageOrigin = 'https://ltex-cmd.github.io';
export function corsHeaders(origin: string | null, development = false): Headers | null {
    const allowed = !origin || origin === pageOrigin || development && /^http:\/\/(127\.0\.0\.1|localhost):\d+$/.test(origin);
    if (!allowed) return null;
    const headers = new Headers({ 'Vary': 'Origin', 'X-Content-Type-Options': 'nosniff' });
    if (origin) {
        headers.set('Access-Control-Allow-Origin', origin);
        headers.set('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
        headers.set('Access-Control-Allow-Headers', 'Content-Type, If-None-Match');
        headers.set('Access-Control-Expose-Headers', 'ETag, X-Atlas-Cache');
        headers.set('Access-Control-Max-Age', '600');
    }
    return headers;
}
