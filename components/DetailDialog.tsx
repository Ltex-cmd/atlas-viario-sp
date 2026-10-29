'use client';
import { useEffect, useRef, type ReactNode } from 'react';
export default function DetailDialog({ children, onClose, focusKey }: {
    children: ReactNode;
    onClose: () => void;
    focusKey: string | number;
}) {
    const ref = useRef<HTMLDialogElement>(null), outside = useRef(false);
    useEffect(() => {
        const dialog = ref.current!, previous = document.activeElement as HTMLElement | null, overflow = document.body.style.overflow;
        dialog.showModal();
        document.body.style.overflow = 'hidden';
        return () => { dialog.close(); document.body.style.overflow = overflow; if (previous?.isConnected)
            previous.focus({ preventScroll: true }); };
    }, []);
    useEffect(() => { const heading = ref.current?.querySelector<HTMLElement>('h2'); if (heading) {
        heading.tabIndex = -1;
        heading.focus({ preventScroll: true });
        ref.current!.scrollTop = 0;
    } }, [focusKey]);
    function isOutside(x: number, y: number) { const r = ref.current!.getBoundingClientRect(); return x < r.left || x > r.right || y < r.top || y > r.bottom; }
    return <dialog ref={ref} className="detail-panel" aria-labelledby="detail-title" onCancel={e => { e.preventDefault(); onClose(); }} onPointerDown={e => { outside.current = e.target === ref.current && isOutside(e.clientX, e.clientY); }} onPointerUp={e => { if (outside.current && isOutside(e.clientX, e.clientY))
        onClose(); outside.current = false; }}>{children}</dialog>;
}
