// Monotone Cubic INL Calibrator (Fritsch–Carlson)
// Usage:
//   MonotoneCubicCalibrator cal;
//   cal.setPoints({0, 1024, 2048, 3072, 4095}, {0.0, 1.01, 2.02, 3.01, 4.00});
//   cal.build();
//   double corrected = cal.apply(measured_raw);

#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cmath>

class MonotoneCubicCalibrator {
public:
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
        h_.resize(m);
        d_.resize(m);
        for (size_t i = 0; i < m; ++i) {
            h_[i] = X_[i+1] - X_[i];
            d_[i] = (Y_[i+1] - Y_[i]) / h_[i];
        }

        // Initial slopes
        M_.assign(n, 0.0);
        M_[0] = endpointSlope0();
        M_[n-1] = endpointSlopeN();

        for (size_t i = 1; i < n - 1; ++i) {
            if (d_[i-1] * d_[i] > 0.0) {
                M_[i] = (d_[i-1] + d_[i]) * 0.5;
            } else {
                M_[i] = 0.0;
            }
        }

        // Fritsch–Carlson limiter to preserve monotonicity
        for (size_t i = 0; i < m; ++i) {
            if (std::abs(d_[i]) < 1e-300) { // flat interval
                M_[i] = 0.0;
                M_[i+1] = 0.0;
                continue;
            }
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
    }

    // Map a raw reading x to corrected value via monotone cubic interpolation
    double apply(double x) const {
        ensureBuilt_();
        // Clamp outside range to end values (or change to linear extrapolation if desired)
        if (x <= X_.front()) return Y_.front();
        if (x >= X_.back())  return Y_.back();

        size_t i = intervalIndex_(x);
        double h = h_[i];
        double t = (x - X_[i]) / h; // in [0,1]

        // Hermite basis
        double t2 = t * t;
        double t3 = t2 * t;
        double h00 = 2*t3 - 3*t2 + 1;
        double h10 = t3 - 2*t2 + t;
        double h01 = -2*t3 + 3*t2;
        double h11 = t3 - t2;

        return h00 * Y_[i] + h10 * h * M_[i] + h01 * Y_[i+1] + h11 * h * M_[i+1];
    }

private:
    std::vector<double> X_, Y_, h_, d_, M_;
    bool built_ = false;

    void ensureBuilt_() const {
        if (!built_) throw std::runtime_error("call build() first");
    }

    size_t intervalIndex_(double x) const {
        // binary search for i such that X_[i] <= x < X_[i+1]
        size_t lo = 0, hi = X_.size() - 1;
        while (hi - lo > 1) {
            size_t mid = (lo + hi) >> 1;
            if (x < X_[mid]) hi = mid; else lo = mid;
        }
        return lo;
    }

    // One-sided endpoint slope with Hyman filter
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


