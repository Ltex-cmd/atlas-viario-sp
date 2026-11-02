import type { TravelMode } from '../lib/journey.ts';

export class JourneyError extends Error {
    status: number;
    constructor(message: string, status = 400) { super(message); this.status = status; }
}

const modeLabel: Record<TravelMode, string> = { foot: 'a pé', bike: 'de bicicleta', car: 'de carro' };

/** OSRM returns route failures as HTTP 400 JSON, not only as HTTP 200.
 * Decode those before classifying an HTTP failure as provider downtime.
 */
export async function readProviderResponse(response: Response, mode?: TravelMode): Promise<any> {
    const service = mode ? 'de rotas' : 'de busca';
    if (response.status === 429) throw new JourneyError(`O serviço ${service} recebeu muitas consultas. Aguarde e tente novamente.`, 429);
    let data: any;
    try { data = await response.json(); } catch { /* Outages may return an HTML proxy error. */ }
    if (mode && (response.ok || response.status === 400)) {
        if (data?.code === 'NoRoute') throw new JourneyError(`Não foi encontrado um caminho ${modeLabel[mode]} entre esses pontos na malha disponível. Ajuste a origem ou o destino, ou tente outro modo de transporte.`, 422);
        if (data?.code === 'NoSegment') throw new JourneyError('Não foi encontrada uma via compatível a até 500 m de um dos pontos. Escolha a origem e o destino mais próximos das vias do mapa.', 422);
    }
    if (!response.ok) throw new JourneyError(`O serviço ${service} está indisponível. Tente novamente em instantes.`, 503);
    if (!data || typeof data !== 'object') throw new JourneyError(`O serviço ${service} retornou uma resposta incompleta. Tente novamente.`, 503);
    return data;
}
