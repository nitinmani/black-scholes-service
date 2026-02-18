// utils/BlackScholesUtil.cpp

#include "utils/BlackScholesUtil.h"
#include <boost/math/distributions/normal.hpp>
#include <boost/math/distributions/gamma.hpp>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_eigen.h>
#include <cmath>
#include <algorithm>
#include <limits>
#include <vector>

#if defined(__AVX2__)
  #include <immintrin.h>
  #define BSU_HAS_AVX2 1
#else
  #define BSU_HAS_AVX2 0
#endif

#if defined(__INTEL_COMPILER) || defined(__INTEL_LLVM_COMPILER)
  #define BSU_HAS_SVML 1
#else
  #define BSU_HAS_SVML 0
#endif

#ifndef M_SQRT1_2
#define M_SQRT1_2 0.70710678118654752440
#endif

#ifndef BSU_FORCE_GSL_IN_FAST
#define BSU_FORCE_GSL_IN_FAST 0
#endif

#ifndef BSU_GL_ORDER
#define BSU_GL_ORDER 32
#endif

namespace BlackScholesUtil {

const double INTEGRATION_UPPER_BOUND = std::numeric_limits<double>::infinity();
const int    GSL_WORKSPACE_SIZE      = 1000;

struct IntegrationParams {
    double stock_price;
    double strike_price;
    double volatility;
    double risk_free_rate;
    double holding_period;
    double volatility_around_holding_period;
};

struct GammaParams {
    double alpha;
    double beta;
    double scale;
    boost::math::gamma_distribution<double> gamma_dist;

    GammaParams(double holding_period, double volatility_around_holding_period)
        : alpha(std::max(holding_period * holding_period /
                         std::max(volatility_around_holding_period * volatility_around_holding_period, 1e-6), 1e-6)),
          beta(holding_period / std::max(volatility_around_holding_period * volatility_around_holding_period, 1e-6)),
          scale(1.0 / beta),
          gamma_dist(alpha, scale) {}
};

struct OptimizedIntegrationParams {
    double stock_price;
    double strike_price;
    double volatility;
    double risk_free_rate;
    GammaParams* gamma_params;
    bool is_binary;
};

double calculateStandardCall(double stock_price, double strike_price,
                             double time_to_maturity, double volatility,
                             double risk_free_rate) {

    if (strike_price <= 0) return stock_price;
    if (stock_price <= 0)  return 0.0;

    if (volatility <= 0 || time_to_maturity <= 0) {
        return std::max(0.0, stock_price - strike_price);
    }

    const double X  = strike_price;
    const double rt = std::sqrt(time_to_maturity);
    const double vs = volatility * rt;
    const double d1 = (std::log(stock_price / X) + (risk_free_rate + 0.5 * volatility * volatility) * time_to_maturity) / vs;
    const double d2 = d1 - vs;

    boost::math::normal_distribution<double> normal(0.0, 1.0);
    return stock_price * boost::math::cdf(normal, d1)
           - X * std::exp(-risk_free_rate * time_to_maturity) * boost::math::cdf(normal, d2);
}

double calculateBinaryCall(double stock_price, double strike_price,
                           double time_to_maturity, double volatility,
                           double risk_free_rate) {

    if (strike_price <= 0) return 1.0;
    if (stock_price <= 0)  return 0.0;

    if (volatility <= 0 || time_to_maturity <= 0) {
        return (stock_price > strike_price) ? 1.0 : 0.0;
    }

    const double rt = std::sqrt(time_to_maturity);
    const double vs = volatility * rt;
    const double d1 = (std::log(stock_price / strike_price) + (risk_free_rate + 0.5 * volatility * volatility) * time_to_maturity) / vs;
    const double d2 = d1 - vs;

    boost::math::normal_distribution<double> normal(0.0, 1.0);
    return std::exp(-risk_free_rate * time_to_maturity) * boost::math::cdf(normal, d2);
}

double standardCallWithGammaPDF(double stock_price, double strike_price,
                                double time_to_maturity, double volatility,
                                double risk_free_rate, double holding_period,
                                double volatility_around_holding_period) {

    double time_variance = std::max(volatility_around_holding_period * volatility_around_holding_period, 1e-6);
    double alpha = std::max(holding_period * holding_period / time_variance, 1e-6);
    double beta  = holding_period / time_variance;

    double bs = calculateStandardCall(stock_price, strike_price, time_to_maturity, volatility, risk_free_rate);
    boost::math::gamma_distribution<double> gamma_dist(alpha, 1.0 / beta);
    double pdf = boost::math::pdf(gamma_dist, time_to_maturity);
    return bs * pdf;
}

double gsl_integrand(double t, void *params) {
    IntegrationParams *p = (IntegrationParams*)params;
    return standardCallWithGammaPDF(p->stock_price, p->strike_price, t,
                                    p->volatility, p->risk_free_rate,
                                    p->holding_period, p->volatility_around_holding_period);
}

double binaryCallWithGammaPDF(double stock_price, double strike_price,
                              double time_to_maturity, double volatility,
                              double risk_free_rate, double holding_period,
                              double volatility_around_holding_period) {

    double time_variance = std::max(volatility_around_holding_period * volatility_around_holding_period, 1e-6);
    double alpha = std::max(holding_period * holding_period / time_variance, 1e-6);
    double beta  = holding_period / time_variance;

    double bs = calculateBinaryCall(stock_price, strike_price, time_to_maturity, volatility, risk_free_rate);
    boost::math::gamma_distribution<double> gamma_dist(alpha, 1.0 / beta);
    double pdf = boost::math::pdf(gamma_dist, time_to_maturity);
    return bs * pdf;
}

double gsl_binary_integrand(double t, void *params) {
    IntegrationParams *p = (IntegrationParams*)params;
    return binaryCallWithGammaPDF(p->stock_price, p->strike_price, t,
                                  p->volatility, p->risk_free_rate,
                                  p->holding_period, p->volatility_around_holding_period);
}

namespace {

inline double _fast_norm_cdf(double x) {
    return 0.5 * std::erfc(-x * M_SQRT1_2);
}

inline double _fast_bs_call(double S, double K, double t, double vol, double r) {
    if (K <= 0.0) return S;
    if (S <= 0.0)  return 0.0;
    if (vol <= 0.0 || t <= 0.0) return std::max(0.0, S - K);
    const double rt = std::sqrt(t);
    const double vs = vol * rt;
    const double d1 = (std::log(S / K) + (r + 0.5 * vol * vol) * t) / vs;
    const double d2 = d1 - vs;
    return S * _fast_norm_cdf(d1) - K * std::exp(-r * t) * _fast_norm_cdf(d2);
}

inline double _fast_bs_binary_call(double S, double K, double t, double vol, double r) {
    if (K <= 0.0) return 1.0;
    if (S <= 0.0)  return 0.0;
    if (vol <= 0.0 || t <= 0.0) return (S > K) ? 1.0 : 0.0;
    const double rt = std::sqrt(t);
    const double vs = vol * rt;
    const double d1 = (std::log(S / K) + (r + 0.5 * vol * vol) * t) / vs;
    const double d2 = d1 - vs;
    return std::exp(-r * t) * _fast_norm_cdf(d2);
}

struct GLTable {
    int n;
    double a;
    std::vector<double> x;
    std::vector<double> w;
};
static thread_local GLTable _glt = {0, std::numeric_limits<double>::quiet_NaN(), {}, {}};

inline void _ensure_gl_table(int n, double a) {
    if (_glt.n == n && std::abs(_glt.a - a) <= 0.0) return;

    gsl_matrix* J = gsl_matrix_calloc(n, n);
    for (int i = 0; i < n; ++i) {
        const double di = 2.0 * i + 1.0 + a;
        gsl_matrix_set(J, i, i, di);
        if (i + 1 < n) {
            const double si = std::sqrt((i + 1.0) * (i + 1.0 + a));
            gsl_matrix_set(J, i, i + 1, si);
            gsl_matrix_set(J, i + 1, i, si);
        }
    }

    gsl_vector* eval  = gsl_vector_alloc(n);
    gsl_matrix* evec  = gsl_matrix_alloc(n, n);
    gsl_eigen_symmv_workspace* w = gsl_eigen_symmv_alloc(n);

    gsl_eigen_symmv(J, eval, evec, w);
    gsl_eigen_symmv_free(w);
    gsl_eigen_symmv_sort(eval, evec, GSL_EIGEN_SORT_VAL_ASC);

    _glt.n = n;
    _glt.a = a;
    _glt.x.resize(n);
    _glt.w.resize(n);

    const double mu0 = std::tgamma(a + 1.0);
    for (int j = 0; j < n; ++j) {
        const double xj = gsl_vector_get(eval, j);
        const double v0 = gsl_matrix_get(evec, 0, j);
        _glt.x[j] = xj;
        _glt.w[j] = mu0 * (v0 * v0);
    }

    gsl_matrix_free(evec);
    gsl_vector_free(eval);
    gsl_matrix_free(J);
}

#if BSU_HAS_AVX2
  static inline __m256d vset1(double x){ return _mm256_set1_pd(x); }
  static inline __m256d vloadu(const double* p){ return _mm256_loadu_pd(p); }
  static inline void     vstoreu(double* p, __m256d v){ _mm256_storeu_pd(p, v); }
  static inline __m256d vmul(__m256d a, __m256d b){ return _mm256_mul_pd(a,b); }
  static inline __m256d vadd(__m256d a, __m256d b){ return _mm256_add_pd(a,b); }
  static inline __m256d vsub(__m256d a, __m256d b){ return _mm256_sub_pd(a,b); }
  static inline __m256d vdiv(__m256d a, __m256d b){ return _mm256_div_pd(a,b); }
  static inline __m256d vsqrt(__m256d a){ return _mm256_sqrt_pd(a); }

  static inline __m256d vexp(__m256d a){
  #if BSU_HAS_SVML
      return _mm256_exp_pd(a);
  #else
      alignas(32) double tmp[4]; vstoreu(tmp, a);
      for (int i=0;i<4;++i) tmp[i] = std::exp(tmp[i]);
      return vloadu(tmp);
  #endif
  }
  static inline __m256d verfc(__m256d a){
  #if BSU_HAS_SVML
      return _mm256_erfc_pd(a);
  #else
      alignas(32) double tmp[4]; vstoreu(tmp, a);
      for (int i=0;i<4;++i) tmp[i] = std::erfc(tmp[i]);
      return vloadu(tmp);
  #endif
  }

  static inline double hsum(__m256d v){
      __m256d t = _mm256_hadd_pd(v, v);
      __m128d lo = _mm256_castpd256_pd128(t);
      __m128d hi = _mm256_extractf128_pd(t, 1);
      __m128d s  = _mm_add_pd(lo, hi);
      double out[2];
      _mm_storeu_pd(out, s);
      return out[0];
  }

  static inline __m256d vphi(__m256d x){
      const __m256d c = vset1(-M_SQRT1_2);
      return vmul(vset1(0.5), verfc(vmul(c, x)));
  }

#endif

inline double _gl_price_call_simd(double S, double K, double vol, double r,
                                  double alpha, double beta, int n){
    const double invGamma = 1.0 / std::tgamma(alpha);
    _ensure_gl_table(n, /*a=*/alpha - 1.0);

#if BSU_HAS_AVX2
    const __m256d vS   = vset1(S);
    const __m256d vK   = vset1(K);
    const __m256d vVol = vset1(vol);
    const __m256d vR   = vset1(r);
    const __m256d vB   = vset1(r + 0.5 * vol * vol);
    const __m256d vA   = vset1(std::log(S / K));
    const __m256d vBeta= vset1(beta);

    double sum = 0.0;
    int i = 0;
    for (; i + 3 < n; i += 4) {
        __m256d X  = vloadu(&_glt.x[i]);
        __m256d W  = vloadu(&_glt.w[i]);
        __m256d t  = vdiv(X, vBeta);
        __m256d rt = vsqrt(t);
        __m256d vs = vmul(vVol, rt);

        __m256d num = vadd(vA, vmul(vB, t));
        __m256d d1  = vdiv(num, vs);
        __m256d d2  = vsub(d1, vs);

        __m256d disc = vexp(vmul(vset1(-r), t));
        __m256d phi1 = vphi(d1);
        __m256d phi2 = vphi(d2);

        __m256d term1 = vmul(vS,  phi1);
        __m256d term2 = vmul(vK, vmul(disc, phi2));
        __m256d price = vsub(term1, term2);

        __m256d contrib = vmul(W, price);
        sum += hsum(contrib);
    }
    for (; i < n; ++i) {
        const double t  = _glt.x[i] / beta;
        const double rt = std::sqrt(t);
        const double vs = vol * rt;
        const double d1 = (std::log(S / K) + (r + 0.5 * vol * vol)*t) / vs;
        const double d2 = d1 - vs;
        const double price = S * _fast_norm_cdf(d1) - K * std::exp(-r*t) * _fast_norm_cdf(d2);
        sum += _glt.w[i] * price;
    }
    return invGamma * sum;
#else
    double sum = 0.0;
    for (int i=0;i<n;++i){
        const double t  = _glt.x[i] / beta;
        const double price = _fast_bs_call(S, K, t, vol, r);
        sum += _glt.w[i] * price;
    }
    return invGamma * sum;
#endif
}

inline double _gl_price_binary_simd(double S, double K, double vol, double r,
                                    double alpha, double beta, int n){
    const double invGamma = 1.0 / std::tgamma(alpha);
    _ensure_gl_table(n, /*a=*/alpha - 1.0);

#if BSU_HAS_AVX2
    const __m256d vK   = vset1(K);
    const __m256d vVol = vset1(vol);
    const __m256d vR   = vset1(r);
    const __m256d vB   = vset1(r + 0.5 * vol * vol);
    const __m256d vA   = vset1(std::log(S / K));
    const __m256d vBeta= vset1(beta);

    double sum = 0.0;
    int i = 0;
    for (; i + 3 < n; i += 4) {
        __m256d X  = vloadu(&_glt.x[i]);
        __m256d W  = vloadu(&_glt.w[i]);
        __m256d t  = vdiv(X, vBeta);
        __m256d rt = vsqrt(t);
        __m256d vs = vmul(vVol, rt);

        __m256d num = vadd(vA, vmul(vB, t));
        __m256d d1  = vdiv(num, vs);
        __m256d d2  = vsub(d1, vs);

        __m256d disc = vexp(vmul(vset1(-r), t));
        __m256d phi2 = vphi(d2);
        __m256d price= vmul(disc, phi2);

        __m256d contrib = vmul(W, price);
        sum += hsum(contrib);
    }
    for (; i < n; ++i) {
        const double t = _glt.x[i] / beta;
        const double price = _fast_bs_binary_call(S, K, t, vol, r);
        sum += _glt.w[i] * price;
    }
    return invGamma * sum;
#else
    double sum = 0.0;
    for (int i=0;i<n;++i){
        const double t  = _glt.x[i] / beta;
        const double price = _fast_bs_binary_call(S, K, t, vol, r);
        sum += _glt.w[i] * price;
    }
    return invGamma * sum;
#endif
}

struct _GslFastParams {
    double S, K, vol, r;
    double alpha;
    double beta;
    double lognorm;
    bool   is_binary;
};

static double _gsl_fast_integrand(double t, void* pp){
    const _GslFastParams* p = static_cast<const _GslFastParams*>(pp);
    if (t <= 0.0) return 0.0;

    double price = p->is_binary
        ? _fast_bs_binary_call(p->S, p->K, t, p->vol, p->r)
        : _fast_bs_call       (p->S, p->K, t, p->vol, p->r);

    const double lp = (p->alpha - 1.0) * std::log(t) - p->beta * t + p->lognorm;
    return std::isfinite(lp) ? price * std::exp(lp) : 0.0;
}

static gsl_integration_workspace* _gsl_ws_fast(){
    static thread_local gsl_integration_workspace* w = gsl_integration_workspace_alloc(8192);
    return w;
}

inline double _integrate_gsl_fast_call(double S,double K,double vol,double r,
                                       double alpha,double beta,bool is_binary){
    _GslFastParams P{
        S, K, vol, r,
        alpha, beta,
        alpha * std::log(beta) - std::lgamma(alpha),
        is_binary
    };
    gsl_function F; F.function = &_gsl_fast_integrand; F.params = &P;
    double result = 0.0, error = 0.0;
    gsl_integration_qagiu(&F, 0.0, 1e-9, 1e-9, 8192, _gsl_ws_fast(), &result, &error);
    return result;
}

inline bool _prefer_gsl_for_gamma(double H, double sigmaH, double alpha){
    if (H <= 0.0 || sigmaH <= 0.0) return false;
    const double cv = sigmaH / H;
    return (cv >= 1.5) || (alpha < 0.5);
}

}

double calculateRandomExpirationCall(double stock_price, double strike_price,
                                     double volatility, double risk_free_rate,
                                     double holding_period, double volatility_around_holding_period) {
    if (strike_price <= 0) return stock_price;
    if (stock_price <= 0)  return 0.0;
    if (volatility <= 0 || holding_period <= 0) return std::max(0.0, stock_price - strike_price);

    if (volatility_around_holding_period == 0 ||
        holding_period / std::max(volatility_around_holding_period, 1e-300) >= 50) {
        return _fast_bs_call(stock_price, strike_price, holding_period, volatility, risk_free_rate);
    }

#if BSU_FORCE_GSL_IN_FAST
    const double var_t = std::max(volatility_around_holding_period * volatility_around_holding_period, 1e-12);
    const double alpha = std::max((holding_period * holding_period) / var_t, 1e-12);
    const double beta  = holding_period / var_t;
    return _integrate_gsl_fast_call(stock_price, strike_price, volatility, risk_free_rate,
                                    alpha, beta, /*is_binary=*/false);
#else
    const double var_t  = std::max(volatility_around_holding_period * volatility_around_holding_period, 1e-12);
    const double alpha  = std::max((holding_period * holding_period) / var_t, 1e-12);
    const double beta   = holding_period / var_t;
    const double sigmaH = std::sqrt(var_t);

    if (_prefer_gsl_for_gamma(holding_period, sigmaH, alpha)) {
        return _integrate_gsl_fast_call(stock_price, strike_price, volatility, risk_free_rate,
                                        alpha, beta, /*is_binary=*/false);
    } else {
        return _gl_price_call_simd(stock_price, strike_price, volatility, risk_free_rate,
                                   alpha, beta, BSU_GL_ORDER);
    }
#endif
}

double calculateRandomExpirationBinaryCall(double stock_price, double strike_price,
                                           double volatility, double risk_free_rate,
                                           double holding_period, double volatility_around_holding_period) {
    if (strike_price <= 0) return 1.0;
    if (stock_price <= 0)  return 0.0;
    if (volatility <= 0 || holding_period <= 0) return (stock_price > strike_price) ? 1.0 : 0.0;

    if (volatility_around_holding_period == 0 ||
        holding_period / std::max(volatility_around_holding_period, 1e-300) >= 50) {
        return _fast_bs_binary_call(stock_price, strike_price, holding_period, volatility, risk_free_rate);
    }

#if BSU_FORCE_GSL_IN_FAST
    const double var_t = std::max(volatility_around_holding_period * volatility_around_holding_period, 1e-12);
    const double alpha = std::max((holding_period * holding_period) / var_t, 1e-12);
    const double beta  = holding_period / var_t;
    return _integrate_gsl_fast_call(stock_price, strike_price, volatility, risk_free_rate,
                                    alpha, beta, /*is_binary=*/true);
#else
    const double var_t  = std::max(volatility_around_holding_period * volatility_around_holding_period, 1e-12);
    const double alpha  = std::max((holding_period * holding_period) / var_t, 1e-12);
    const double beta   = holding_period / var_t;
    const double sigmaH = std::sqrt(var_t);

    if (_prefer_gsl_for_gamma(holding_period, sigmaH, alpha)) {
        return _integrate_gsl_fast_call(stock_price, strike_price, volatility, risk_free_rate,
                                        alpha, beta, /*is_binary=*/true);
    } else {
        return _gl_price_binary_simd(stock_price, strike_price, volatility, risk_free_rate,
                                     alpha, beta, BSU_GL_ORDER);
    }
#endif
}

std::vector<double> calculateMultipleStandardCalls(const std::vector<double>& stock_prices,
                                                  const std::vector<double>& strike_prices,
                                                  const std::vector<double>& time_to_maturities,
                                                  const std::vector<double>& volatilities,
                                                  const std::vector<double>& risk_free_rates) {
    size_t n = stock_prices.size();
    std::vector<double> results(n);
    
    for (size_t i = 0; i < n; ++i) {
        results[i] = calculateStandardCall(stock_prices[i], strike_prices[i],
                                          time_to_maturities[i], volatilities[i], risk_free_rates[i]);
    }
    return results;
}

std::vector<double> calculateMultipleBinaryCalls(const std::vector<double>& stock_prices,
                                                const std::vector<double>& strike_prices,
                                                const std::vector<double>& time_to_maturities,
                                                const std::vector<double>& volatilities,
                                                const std::vector<double>& risk_free_rates) {
    size_t n = stock_prices.size();
    std::vector<double> results(n);
    
    for (size_t i = 0; i < n; ++i) {
        results[i] = calculateBinaryCall(stock_prices[i], strike_prices[i],
                                        time_to_maturities[i], volatilities[i], risk_free_rates[i]);
    }
    return results;
}

std::vector<double> calculateMultipleRandomExpirationCalls(const std::vector<double>& stock_prices,
                                                          const std::vector<double>& strike_prices,
                                                          const std::vector<double>& volatilities,
                                                          const std::vector<double>& risk_free_rates,
                                                          const std::vector<double>& holding_periods,
                                                          const std::vector<double>& volatility_around_holding_periods) {
    size_t n = stock_prices.size();
    std::vector<double> results(n);
    
    for (size_t i = 0; i < n; ++i) {
        results[i] = calculateRandomExpirationCall(stock_prices[i], strike_prices[i],
                                                  volatilities[i], risk_free_rates[i],
                                                  holding_periods[i], volatility_around_holding_periods[i]);
    }
    return results;
}

std::vector<double> calculateMultipleRandomExpirationBinaryCalls(const std::vector<double>& stock_prices,
                                                                const std::vector<double>& strike_prices,
                                                                const std::vector<double>& volatilities,
                                                                const std::vector<double>& risk_free_rates,
                                                                const std::vector<double>& holding_periods,
                                                                const std::vector<double>& volatility_around_holding_periods) {
    size_t n = stock_prices.size();
    std::vector<double> results(n);
    
    for (size_t i = 0; i < n; ++i) {
        results[i] = calculateRandomExpirationBinaryCall(stock_prices[i], strike_prices[i],
                                                        volatilities[i], risk_free_rates[i],
                                                        holding_periods[i], volatility_around_holding_periods[i]);
    }
    return results;
}

} // namespace BlackScholesUtil
