// Rendered vector tiles can round coordinates or return the same ID more than
// once. Resolve the hit against the original API response before querying it.
export function resolvePointSelection(rendered: any[], collection: any) {
    const ids = [...new Set<number>(rendered.filter(feature => Number(feature.properties?.count) === 1)
        .map(feature => Number(feature.properties.id)).filter(id => Number.isSafeInteger(id) && id > 0))];
    if (!ids.length) return null;
    const official = new Map<number, any>((collection?.features || []).filter((feature: any) => Number(feature.properties?.count) === 1)
        .map((feature: any) => [Number(feature.properties.id), feature]));
    const points = ids.map(id => official.get(id));
    if (points.some(point => !point || point.geometry?.type !== 'Point')) return null;
    const coordinates = points.map(point => point.geometry.coordinates as [number, number]);
    if (coordinates.some(point => point.length !== 2 || point.some(value => !Number.isFinite(value)))) return null;
    const xs = coordinates.map(point => point[0]), ys = coordinates.map(point => point[1]);
    return { ids, count: ids.length, bbox: [Math.min(...xs) - 1e-8, Math.min(...ys) - 1e-8, Math.max(...xs) + 1e-8, Math.max(...ys) + 1e-8].join(',') };
}

export async function loadPointSelection(ids: number[], fetchPage: (offset: number, limit: number) => Promise<any>) {
    const remaining = new Set(ids), selected: any[] = [];
    for (let offset = 0; remaining.size && offset < 12000;) {
        const page = await fetchPage(offset, 200);
        for (const item of page.items) if (remaining.delete(Number(item.id))) selected.push(item);
        offset += page.items.length;
        if (!page.has_more || !page.items.length) break;
    }
    if (remaining.size) throw Error('Não foi possível recuperar todos os pontos selecionados. Aproxime a área e selecione novamente.');
    return selected;
}
