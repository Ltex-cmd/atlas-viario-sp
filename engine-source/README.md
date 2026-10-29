# Snapshot do motor C

`engine.c` e `engine.h` são cópias de publicação geradas por `visualizador-web/tools/build_wasm.sh` no projeto completo. O código canônico permanece em `visualizador-web/backend/`. Este snapshot torna verificável qual algoritmo produz os resultados hospedados e não contém cópias dos módulos acadêmicos originais.

O módulo é um reactor WASI, compilado com clang do WASI SDK 34, `-O3 -mexec-model=reactor`, memória inicial 393216 e máxima 67108864. Exporta alocação, ciclo de vida do motor, acesso por ID, buffers de filtros/estatísticas/agrupamentos e `run_query` / `run_select`. Record=48 bytes, Filter=80 bytes; o contrato é consumido por `worker/api.ts`.
