// Implements WriteWaterPressure() which filters a list of water pressure
// points and writes the unique ones to a file.
//
// A point is skipped if it is directly close to an already-kept point,
// where "close" means |dx| < TOLERANCE and |dy| < TOLERANCE.
//
// Internally uses a spatial grid to look up nearby kept points in
// O(log n) per point, giving O(n log n) overall.
// See README.md for a detailed explanation of the algorithm.

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <iomanip>
#include <limits>

constexpr double      TOLERANCE = 1e-3;
constexpr const char* OUTPUT_FILENAME = R"(C:\Users\nthan\OneDrive\Documents\output.txt)";

struct WaterPressurePoint
{
    double x;
    double y;
    double pressure;
};

// Bucket kept points by floor(x / TOLERANCE) for fast proximity lookup.
// Each bucket stores {y, x} pairs in a std::set, which sorts them
// lexicographically by (y, x) automatically, so we can binary-search
// the y-band with lower/upper_bound using y as the primary key.
using YXPair = std::pair<double, double>;
using Bucket = std::set<YXPair>;
using Grid = std::map<long long, Bucket>;

// Returns true if (cx, cy) is directly close to any already-kept point,
// i.e. there exists a kept point where |dx| < TOLERANCE and |dy| < TOLERANCE.
inline bool IsClose(const Grid& grid, double cx, double cy)
{
    // Any point with |dx| < TOLERANCE must live in one of 3 adjacent buckets
    const long long bx = static_cast<long long>(std::floor(cx / TOLERANCE));

    for (long long b = bx - 1; b <= bx + 1; ++b)
    {
        auto mapIt = grid.find(b);
        if (mapIt == grid.end()) continue;

        const Bucket& bucket = mapIt->second;

        // Binary-search the y-band with lower/upper_bound using y as the primary key to avoid scanning the entire bucket
        auto lo = bucket.lower_bound({ cy - TOLERANCE, -std::numeric_limits<double>::infinity() });
        auto hi = bucket.upper_bound({ cy + TOLERANCE,  std::numeric_limits<double>::infinity() });

        // At most 6 candidates by the packing property
        for (auto it = lo; it != hi; ++it)
        {
            if (std::fabs(it->second - cx) < TOLERANCE && std::fabs(it->first - cy) < TOLERANCE)
                return true;
        }
    }

    return false;
}

void WriteWaterPressure(std::vector<WaterPressurePoint>& points)
{
    if (points.size() == 0)
        return;

    std::ofstream out(OUTPUT_FILENAME);
    if (!out.is_open())
    {
        std::cerr << "Cannot open file: " << OUTPUT_FILENAME << "\n";
        return;
    }
    out << std::fixed << std::setprecision(3);

    Grid grid;  // spatial grid of kept points
    for (const auto& point : points)
    {
        const double    cx = point.x;
        const double    cy = point.y;
        const long long bx = static_cast<long long>(std::floor(cx / TOLERANCE));

        // Only write and keep track of this point if it is not close to any
        // already-kept point
        if (!IsClose(grid, cx, cy))
        {
            grid[bx].insert({ cy, cx });
            out << cx << " " << cy << " " << point.pressure << "\n";
        }
    }
}

int main()
{
    std::vector<WaterPressurePoint> points = {
        { 0,     0,    0  },
        { 0,     0,    15 },   
        { 1e-19, 0,    0  },   
        { 0.5,     0,    25.12345  },
        { 2,     0,    10 },
        { 1,     1,    7  },
    };

    WriteWaterPressure(points);

    return 0;
}
