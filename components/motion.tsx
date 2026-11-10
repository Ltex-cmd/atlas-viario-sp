'use client';
import { useEffect, useLayoutEffect, useRef, useState } from 'react';

const useBeforePaint = typeof window === 'undefined' ? useEffect : useLayoutEffect;
export const reducedMotion = () => typeof matchMedia === 'function' && matchMedia('(prefers-reduced-motion: reduce)').matches;
const observable = () => typeof IntersectionObserver === 'function' && !reducedMotion();

// Ref callback: marks the element so app/motion.css reveals it when it enters the screen.
export function reveal(element: HTMLElement | null) {
    if (!element || !observable()) return;
    element.dataset.reveal = 'out';
    const observer = new IntersectionObserver(entries => {
        if (!entries.some(entry => entry.isIntersecting)) return;
        element.dataset.reveal = 'in';
        observer.disconnect();
    }, { rootMargin: '0px 0px -8% 0px' });
    observer.observe(element);
    return () => observer.disconnect();
}

// Whole numbers count from the value on screen to the new one. Without a browser,
// or with reduced motion, the final value is rendered directly.
export function CountUp({ value, duration = 800, format = (n: number) => n.toLocaleString('pt-BR'), announce = false }: {
    value: number; duration?: number; format?: (n: number) => string; announce?: boolean;
}) {
    const [shown, setShown] = useState(value);
    const element = useRef<HTMLSpanElement>(null), current = useRef(value), target = useRef(value);
    const frame = useRef(0), visible = useRef(false), started = useRef(false);
    target.current = value;
    const run = () => {
        cancelAnimationFrame(frame.current);
        const from = current.current, to = target.current, begin = performance.now();
        if (from === to) return;
        const tick = (now: number) => {
            const t = Math.min(1, (now - begin) / duration), eased = 1 - (1 - t) ** 4;
            current.current = t < 1 ? Math.round(from + (to - from) * eased) : to;
            setShown(current.current);
            if (t < 1) frame.current = requestAnimationFrame(tick);
        };
        frame.current = requestAnimationFrame(tick);
    };
    useBeforePaint(() => {
        if (!observable()) { current.current = value; setShown(value); return; }
        if (!started.current) { started.current = true; current.current = 0; setShown(0); }
        if (visible.current) run();
        return () => cancelAnimationFrame(frame.current);
    }, [value]);
    useEffect(() => {
        if (!element.current || !observable()) return;
        const observer = new IntersectionObserver(entries => {
            if (!entries.some(entry => entry.isIntersecting)) return;
            visible.current = true; observer.disconnect(); run();
        });
        observer.observe(element.current);
        return () => observer.disconnect();
    }, []);
    // Inside live regions only the final value is exposed to assistive technology.
    if (announce) return <><span ref={element} aria-hidden="true">{format(shown)}</span><span className="visually-hidden">{format(value)}</span></>;
    return <span ref={element}>{format(shown)}</span>;
}
