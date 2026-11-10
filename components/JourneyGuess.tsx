import { useState } from 'react';

export default function JourneyGuess({ radius, period, onReveal }: { radius: number; period: string; onReveal: (guess?: number) => void }) {
    const [value, setValue] = useState('');
    const valid = /^\d+$/.test(value) && Number.isSafeInteger(Number(value));
    return <form className="journey-guess" onSubmit={e => { e.preventDefault(); if (valid) onReveal(Number(value)); }}>
        <label htmlFor="journey-guess">Quantos sinistros você acha que foram registrados perto desse caminho?</label>
        <p className="journey-note">Até {radius} m do percurso · {period}</p>
        <div className="journey-guess-actions">
            <input id="journey-guess" type="number" min="0" step="1" inputMode="numeric" value={value} onChange={e => setValue(e.target.value)} placeholder="Seu chute"/>
            <button className="primary-button" disabled={!valid}>Revelar</button>
            <button type="button" className="journey-text-button" onClick={() => onReveal()}>Ver sem chutar</button>
        </div>
    </form>;
}
