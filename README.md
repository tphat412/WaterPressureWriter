# Water Pressure Writer

Filters a list of water pressure points by removing duplicates based on spatial proximity, then writes the unique points to a file.

## Duplicate Rule

A point is removed if it is **directly close** to an already-kept point, where "close" means:

```
|dx| < TOLERANCE  AND  |dy| < TOLERANCE
```

Only the **first occurrence** in the input is kept. Transitivity is **not** applied — a point is only removed if it is directly close to a kept point, not a removed one.

### Transitivity Example

```
TOLERANCE = 1e-3

A (0.0000, 0)  →  no kept points yet          →  kept
B (0.0006, 0)  →  |A-B| = 0.0006 < tol        →  removed  (close to kept A)
C (0.0012, 0)  →  |B-C| = 0.0006 < tol        →  kept     (B was removed, not kept)
               →  |A-C| = 0.0012 > tol
```

C survives because it is only close to B, which was already removed. If transitivity were applied, C would be removed — but that could cause a long chain of points to be eliminated even if the first and last are far apart.

## Algorithm — O(n log n) Guaranteed

### The Packing Property

The kept set has a geometric constraint: no two kept points can both be within `TOLERANCE` in x **and** within `TOLERANCE` in y (otherwise one would have been removed). This means at most **one** kept point can exist inside any `TOLERANCE × TOLERANCE` cell.

### Spatial Grid

Instead of checking every kept point for each new point (O(n²)), we use a spatial grid:

1. **Bucket by x** — assign each kept point to bucket `bx = floor(x / TOLERANCE)`
2. **Query 3 buckets** — any point with `|dx| < TOLERANCE` from `cx` must live in bucket `bx-1`, `bx`, or `bx+1`
3. **Binary search by y** — inside each bucket, points are stored in a `std::set` sorted by `(y, x)`. Use `lower_bound` / `upper_bound` to find the y-band `[cy - tol, cy + tol]` in O(log n)
4. **Scan candidates** — by the packing property, the query window (`3*TOL wide × 2*TOL tall`) contains at most `3 × 2 = 6` cells of size `TOL × TOL`, so **at most 6 candidates** — a constant regardless of n

### Complexity

| Step | Cost |
|---|---|
| Iterate through all n points | O(n) |
| Query 3 buckets per point | O(1) — always exactly 3 buckets |
| Binary search per point | O(log n) |
| Candidate scan per point | O(1) — at most 6 by packing property |
| **Total** | **O(n log n) — guaranteed** |
