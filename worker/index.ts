import { handleAPI } from './api';
import { corsHeaders } from './public-api';

export default {
    async fetch(request: Request, env: any) {
        const url = new URL(request.url);
        const cors = corsHeaders(request.headers.get('Origin'), import.meta.env.DEV);
        if (!cors) return Response.json({ error: 'Origem não permitida.' }, { status: 403 });
        if (request.method === 'OPTIONS') return new Response(null, { status: 204, headers: cors });
        let response: Response;
        if (url.pathname.startsWith('/api/v1/')) response = await handleAPI(request, env);
        else if (url.pathname.startsWith('/data/')) response = new Response('Use a API de consulta.', { status: 403 });
        else response = Response.json({ service: 'Atlas Viário API', frontend: 'https://ltex-cmd.github.io/atlas-viario-sp/', source: 'Infosiga / Detran-SP', status: 'ok' });
        const headers = new Headers(response.headers);
        cors.forEach((value, name) => headers.set(name, value));
        return new Response(response.body, { status: response.status, statusText: response.statusText, headers });
    },
};
