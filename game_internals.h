#pragma once
#define HISTORY_SIZE   10
#define CELLS_COUNT    4
// Highest representable cell exponent (2^16 = 65536): the digits sprite atlas
// has exactly this many entries, and loaded saves are validated against it.
#define MAX_CELL_VALUE 16
