import React from 'react';
import { createRoot } from 'react-dom/client';
import Atlas from '../components/Atlas';
import '../styles/atlas.css';
import 'maplibre-gl/dist/maplibre-gl.css';

createRoot(document.getElementById('root')!).render(<Atlas />);
