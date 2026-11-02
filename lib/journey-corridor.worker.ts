/// <reference lib="webworker" />
import { journeyCorridor } from './journey-corridor';
import type { Coordinate } from './journey';

const scope = self as unknown as DedicatedWorkerGlobalScope;
scope.onmessage = (event: MessageEvent<{ coordinates: Coordinate[]; radius: number }>) => {
    scope.postMessage(journeyCorridor(event.data.coordinates, event.data.radius));
};
