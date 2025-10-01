/*
  Program: Closest Points (Divide and Conquer)
  Files:   closest_points.cpp, utility.h, points.txt
  Input:   points.txt containing one "x y" pair per line (decimal ok)
  Output:  The two closest points and the distance between them

  Approach (as discussed in class):
    1) Read all points from points.txt into a vector<Point>.
    2) Create copies sorted by x (Px) and by y (Py).
    3) Use a recursive divide-and-conquer algorithm:
         - Split Px at the midpoint x into XL and XR.
         - Split Py into YL and YR by comparing each point's x with the midpoint x.
         - Recurse on left/right to get best pairs and delta = min(dL, dR).
         - Build the "strip" of points within delta of the midpoint x; examine
           only the next few neighbors in y-order (<= 7 comparisons per point).
    4) Return the best pair and print it.

  Complexity: O(n log n)
*/

#include "utility.h"

// ------------------------------ Data model ------------------------------

struct Point {
    double x;
    double y;
};

struct ClosestResult {
    Point a;
    Point b;
    double dist;
};

// Euclidean distance between two points
static inline double dist(const Point& p, const Point& q) {
    const double dx = p.x - q.x;
    const double dy = p.y - q.y;
    return std::sqrt(dx*dx + dy*dy);
}

// ------------------------------ Helpers ---------------------------------

// Brute force for very small n (n <= 3 is standard and sufficient)
static ClosestResult bruteForce(const vector<Point>& pts) {
    ClosestResult best{{0,0},{0,0}, std::numeric_limits<double>::infinity()};
    const size_t n = pts.size();
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            double d = dist(pts[i], pts[j]);
            if (d < best.dist) {
                best = {pts[i], pts[j], d};
            }
        }
    }
    return best;
}

// Merge step restricted to the vertical "strip" around midpoint.
// Points are already sorted by y. Only check up to next 7 neighbors.
static ClosestResult closestInStrip(const vector<Point>& strip, double delta) {
    ClosestResult best{{0,0},{0,0}, delta};  // initialize with current best
    const size_t n = strip.size();
    for (size_t i = 0; i < n; ++i) {
        // Compare with next points while y-distance < best.dist
        for (size_t j = i + 1; j < n; ++j) {
            if ((strip[j].y - strip[i].y) >= best.dist) break;
            double d = dist(strip[i], strip[j]);
            if (d < best.dist) {
                best = {strip[i], strip[j], d};
            }
        }
    }
    return best;
}

// Core recursive routine.
// Px: points sorted by x
// Py: points sorted by y
static ClosestResult closestRec(const vector<Point>& Px, const vector<Point>& Py) {
    const size_t n = Px.size();

    // Base: small subproblem
    if (n <= 3) {
        // bruteForce expects the small set; order doesn't matter
        vector<Point> small = Px;
        // Ensure return includes precise min pair
        return bruteForce(small);
    }

    // Split around midpoint in Px
    const size_t mid = n / 2;
    const Point midPoint = Px[mid]; // its x defines the vertical split

    vector<Point> XL(Px.begin(), Px.begin() + mid);
    vector<Point> XR(Px.begin() + mid, Px.end());

    // Partition Py into YL, YR by comparing x to midpoint's x
    vector<Point> YL; YL.reserve(XL.size());
    vector<Point> YR; YR.reserve(XR.size());
    for (const Point& p : Py) {
        if (p.x < midPoint.x || (p.x == midPoint.x && p.y <= midPoint.y)) {
            // Tie-breaker puts points on the "left" consistently
            if (!XL.empty() && (p.x < midPoint.x || (&p != &midPoint))) {
                YL.push_back(p);
            } else if (p.x < midPoint.x) {
                YL.push_back(p);
            } else {
                // Point equal to mid may end up left or right; handle below by containment
                // We’ll just push_left if it exists in XL; otherwise right.
                // But since Py elements are copies, we can use membership by x cut:
                if (p.x <= midPoint.x) YL.push_back(p);
                else YR.push_back(p);
            }
        } else {
            YR.push_back(p);
        }
    }

    // Recurse
    ClosestResult leftBest  = closestRec(XL, YL);
    ClosestResult rightBest = closestRec(XR, YR);

    // Current best delta
    ClosestResult best = (leftBest.dist < rightBest.dist) ? leftBest : rightBest;
    double delta = best.dist;

    // Build strip: points within delta of vertical line x = midPoint.x, in y-order
    vector<Point> strip;
    strip.reserve(n);
    for (const Point& p : Py) {
        if (std::fabs(p.x - midPoint.x) < delta) strip.push_back(p);
    }

    // Check strip neighbors
    ClosestResult stripBest = closestInStrip(strip, delta);
    if (stripBest.dist < best.dist) best = stripBest;

    return best;
}

// Convenience wrapper: sorts once (x and y) then calls recursion
static ClosestResult closestPair(vector<Point> pts) {
    if (pts.size() < 2) {
        return {{0,0},{0,0}, std::numeric_limits<double>::infinity()};
    }

    vector<Point> Px = pts;
    vector<Point> Py = pts;

    std::sort(Px.begin(), Px.end(), [](const Point& a, const Point& b) {
        if (a.x != b.x) return a.x < b.x;
        return a.y < b.y;
    });
    std::sort(Py.begin(), Py.end(), [](const Point& a, const Point& b) {
        if (a.y != b.y) return a.y < b.y;
        return a.x < b.x;
    });

    return closestRec(Px, Py);
}

// ------------------------------ I/O -------------------------------------

// Reads "points.txt" with one "x y" per line.
static bool readPoints(const string& filename, vector<Point>& out) {
    ifstream fin(filename);
    if (!fin) return false;

    double x, y;
    while (fin >> x >> y) {
        out.push_back(Point{x, y});
    }
    return true;
}

// ------------------------------ main ------------------------------------

int main() {
    // 1) Read input
    vector<Point> pts;
    const string filename = "points.txt";
    if (!readPoints(filename, pts)) {
        cerr << "Error: could not open '" << filename
             << "'. Make sure it is in the working directory.\n";
        return 1;
    }

    if (pts.size() < 2) {
        cout << "Need at least two points.\n";
        return 0;
    }

    // 2) Solve
    ClosestResult ans = closestPair(pts);

    // 3) Output (fixed decimals for readability)
    cout.setf(ios::fixed); 
    cout << setprecision(6);

    cout << "Closest points:\n";
    cout << "  P1 = (" << ans.a.x << ", " << ans.a.y << ")\n";
    cout << "  P2 = (" << ans.b.x << ", " << ans.b.y << ")\n";
    cout << "Distance: " << ans.dist << "\n";

    return 0;
}
