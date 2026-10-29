// Cache only completed data. A request's pending I/O must never hold up another
// request: Workers may discard its continuations when the client disconnects.
export function createPartitionReader<T>(open: (bytes: ArrayBuffer, previous: T | null) => T) {
    let cached: { year: number; state: T } | null = null;
    return async function read<R>(year: number, fetchBytes: () => Promise<ArrayBuffer>, consume: (state: T, warm: boolean) => R): Promise<R> {
        if (cached?.year === year) return consume(cached.state, true);
        const bytes = await fetchBytes();
        let warm = true;
        if (cached?.year !== year) {
            const previous = cached?.state ?? null;
            cached = null;
            const state = open(bytes, previous);
            cached = { year, state };
            warm = false;
        }
        // No await between replacing the shared partition and consuming it.
        // Both open and consume must be synchronous; no pending promise is cached.
        return consume(cached.state, warm);
    };
}
