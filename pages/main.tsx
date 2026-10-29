import React from 'react';
import { createRoot } from 'react-dom/client';
import Atlas from '../components/Atlas';
import '../app/globals.css';
import 'maplibre-gl/dist/maplibre-gl.css';

createRoot(document.getElementById('root')!).render(<Atlas />);
