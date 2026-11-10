'use client';
import { useEffect, useRef, type ReactNode } from 'react';
import { Drawer, DrawerBody, DrawerHeader } from './boardui/base/drawer/drawer';

export default function DetailDialog({ children, onClose, focusKey, title = 'Detalhes do sinistro', wide = false }: {
    children: ReactNode; onClose: () => void; focusKey: string | number; title?: string; wide?: boolean;
}) {
    const body = useRef<HTMLDivElement>(null);
    useEffect(() => {
        const frame = requestAnimationFrame(() => {
            const heading = body.current?.querySelector<HTMLElement>('h2');
            if (heading) { heading.tabIndex = -1; heading.focus({ preventScroll: true }); }
            else body.current?.focus({ preventScroll: true });
            body.current?.parentElement?.scrollTo({ top: 0 });
        });
        return () => cancelAnimationFrame(frame);
    }, [focusKey]);
    return <Drawer placement="end" size={wide ? 'xl' : 'lg'} isOpen onOpenChange={open => { if (!open) onClose(); }} className="atlas-detail-drawer" aria-label={title}>
        <DrawerHeader title={title} closeLabel="Fechar detalhes"/>
        <DrawerBody className="atlas-detail-body"><div ref={body} tabIndex={-1} className="atlas-detail-content">{children}</div></DrawerBody>
    </Drawer>;
}
