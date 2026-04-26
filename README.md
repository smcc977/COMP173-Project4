# COMP173-Project4 — MMU Simulator

## Files

| File | Description |
|------|-------------|
| `mmu.c` | Main source file. Implements the Memory Management Unit simulator including the TLB, page table, backing store allocation, and address translation logic. |
| `BACKING_STORE.bin` | Binary file simulating the swap area (disk). Each page is 256 bytes; the simulator reads pages from this file on a page fault. |
| `addresses.txt` | Input file containing logical addresses (one per line) to be translated by the simulator. |
| `out.txt` | Output file contains the same trace as stdout. |
| `Makefile` | Build file (targets Linux kernel module build; use the manual `gcc` command below on other systems). |

## Dependencies

`mmu.c` depends on:
- `BACKING_STORE.bin` — must be present and passed as the first argument at runtime
- `addresses.txt` (or any logical address file) — passed as the second argument at runtime
- Standard C libraries: `stdio.h`, `stdlib.h`, `stdint.h`, `stdbool.h`

## Compile

```bash
gcc mmu.c -o mmu
```

## Execute

```bash
./mmu BACKING_STORE.bin addresses.txt > out.txt
```

The program writes a full address trace to both stdout and `out.txt`, then prints aggregate statistics:

```
Aggregate page faults = 244
Aggregate TLB hits = 55
```