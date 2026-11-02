import type { Metadata } from 'next';
import '../styles/atlas.css';
import 'maplibre-gl/dist/maplibre-gl.css';
export const metadata: Metadata={title:'Atlas Viário · São Paulo',description:'Consulte os sinistros de trânsito de São Paulo por município, período, tipo de via e transporte. Dados do Infosiga.',icons:{icon:'/favicon.svg'},openGraph:{title:'Atlas Viário · São Paulo',description:'Mapa de sinistros de trânsito e comparação de algoritmos de consulta.',locale:'pt_BR',type:'website'},robots:{index:false,follow:false}};
export default function RootLayout({children}:{children:React.ReactNode}){return <html lang="pt-BR"><body>{children}</body></html>}
