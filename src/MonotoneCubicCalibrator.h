// Monotone Cubic INL Calibrator (Fritsch–Carlson)
// Build maps X -> Y with monotonic cubic; optional input bounds (min/max) to skip correction.
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>

// #define MONOTONE_CUBIC_DEBUG

class MonotoneCubicCalibrator {
public:
    // Optional bounds on input domain; when set, apply(x) returns x unchanged outside [min,max].
    // They auto-initialize to [X_.front(), X_.back()] after build(), but you can override.
    double min = std::numeric_limits<double>::quiet_NaN();
    double max = std::numeric_limits<double>::quiet_NaN();

    void setPoints(const std::vector<double>& x, const std::vector<double>& y) {
        X_ = x; Y_ = y; built_ = false;
    }

    void build() {
        const size_t n = X_.size();
        if (n < 2 || Y_.size() != n) throw std::runtime_error("need >=2 points");
        for (size_t i = 1; i < n; ++i) {
            if (!(X_[i] > X_[i-1])) throw std::runtime_error("x must be strictly increasing");
        }

        const size_t m = n - 1;
        h_.resize(m); d_.resize(m);
        for (size_t i = 0; i < m; ++i) {
            h_[i] = X_[i+1] - X_[i];
            d_[i] = (Y_[i+1] - Y_[i]) / h_[i];
        }

        // Initial slopes
        M_.assign(n, 0.0);
        M_[0]    = endpointSlope0();
        M_[n-1]  = endpointSlopeN();
        for (size_t i = 1; i < n - 1; ++i)
            M_[i] = (d_[i-1] * d_[i] > 0.0) ? 0.5 * (d_[i-1] + d_[i]) : 0.0;

        // Fritsch–Carlson limiter
        for (size_t i = 0; i < m; ++i) {
            if (std::abs(d_[i]) < 1e-300) { M_[i] = 0.0; M_[i+1] = 0.0; continue; }
            double a = M_[i]     / d_[i];
            double b = M_[i + 1] / d_[i];
            double s = a*a + b*b;
            if (s > 9.0) {
                double t = 3.0 / std::sqrt(s);
                M_[i]     = t * a * d_[i];
                M_[i + 1] = t * b * d_[i];
            }
        }

        built_ = true;

        // Auto-bounds to first/last knot if not already set
        if (!std::isfinite(min)) min = X_.front();
        if (!std::isfinite(max)) max = X_.back();

#ifdef MONOTONE_CUBIC_DEBUG
        printKnotTable();
        printSampleTable((X_.back()-X_.front()) / (n*2.0));
#endif
    }

    // Map x via monotone cubic; outside [min,max] return x unchanged (no correction).
    double apply(double x) const {
        ensureBuilt_();

        // Skip correction outside optional bounds
        if (std::isfinite(min) && x < min) return x;
        if (std::isfinite(max) && x > max) return x;

        // Still guard against numerical strays at ends
        if (x <= X_.front()) return Y_.front();
        if (x >= X_.back())  return Y_.back();

        size_t i = intervalIndex_(x);
        double h = h_[i];
        double t = (x - X_[i]) / h; // in [0,1]

        // Hermite basis
        double t2 = t * t, t3 = t2 * t;
        double h00 =  2*t3 - 3*t2 + 1;
        double h10 =      t3 - 2*t2 + t;
        double h01 = -2*t3 + 3*t2;
        double h11 =      t3 -    t2;

        return h00 * Y_[i] + h10 * h * M_[i] + h01 * Y_[i+1] + h11 * h * M_[i+1];
    }

    // ---------- Debug helpers ----------
    void printKnotTable() const {
        ensureBuilt_();
        std::printf("\n[MonotoneCubicCalibrator] Knot Table   (bounds: [%g, %g])\n", min, max);
        std::printf("%-4s %-14s %-14s %-14s %-14s %-14s\n",
                    "i", "X[i]", "Y[i]", "h[i]", "d[i]", "M[i]");
        const size_t n = X_.size();
        for (size_t i = 0; i < n; ++i) {
            const bool hasSeg = (i < n-1);
            std::printf("%-4zu % -14.8f % -14.8f % -14.8f % -14.8f % -14.8f\n",
                        i, X_[i], Y_[i],
                        hasSeg ? h_[i] : NAN,
                        hasSeg ? d_[i] : NAN,
                        M_[i]);
        }
    }

    void printSampleTable(double step) const {
        ensureBuilt_();
        if (step <= 0) return;
        const double xmin = X_.front();
        const double xmax = X_.back();
        std::printf("\n[MonotoneCubicCalibrator] Samples (x, apply(x))\n");
        std::printf("%-14s %-14s\n", "x", "apply(x)");
        for (double x = xmin; x <= xmax; x += step)
            std::printf("% -14.8f % -14.8f\n", x, apply(x));
        if (std::abs((xmax - xmin)) / step > 1e-9)
            std::printf("% -14.8f % -14.8f\n", xmax, apply(xmax));
    }

private:
    std::vector<double> X_, Y_, h_, d_, M_;
    bool built_ = false;

    void ensureBuilt_() const {
        if (!built_) throw std::runtime_error("call build() first");
    }

    size_t intervalIndex_(double x) const {
        size_t lo = 0, hi = X_.size() - 1;
        while (hi - lo > 1) {
            size_t mid = (lo + hi) >> 1;
            if (x < X_[mid]) hi = mid; else lo = mid;
        }
        return lo;
    }

    // One-sided endpoint slopes with Hyman filter
    double endpointSlope0() const {
        if (X_.size() == 2) return d_[0];
        double h0 = h_[0], h1 = h_[1];
        double d0 = d_[0], d1 = d_[1];
        double m0 = ((2*h0 + h1)*d0 - h0*d1) / (h0 + h1);
        if (m0 * d0 <= 0) return 0.0;
        if (std::abs(m0) > 3*std::abs(d0)) return 3*d0;
        return m0;
    }
    double endpointSlopeN() const {
        size_t n = X_.size();
        if (n == 2) return d_[0];
        double hm1 = h_[n-2], hm2 = h_[n-3];
        double dm1 = d_[n-2], dm2 = d_[n-3];
        double mn = ((2*hm1 + hm2)*dm1 - hm1*dm2) / (hm1 + hm2);
        if (mn * dm1 <= 0) return 0.0;
        if (std::abs(mn) > 3*std::abs(dm1)) return 3*dm1;
        return mn;
    }
};
