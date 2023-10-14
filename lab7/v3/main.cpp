#include <iostream>
#include <cstdint>

using namespace std;

int log_10(int64_t x) {
    if (x < 10) return 1;
    return 1 + log_10(x / 10);
}

int64_t pow_10(int64_t p) {
    if (p <= 0) return 1;
    return 10 * pow_10(p - 1);
}

int get_digit(int64_t n, size_t index) {
    return (n / pow_10(log_10(n) - 1 - index)) % 10;
}

int main() {
    int64_t n, m, dp[log_10(INT64_MAX) + 1];
    while (cin >> n >> m) {
        int64_t l = 1;                        // max 10 pow less prefix of N (ex: n = 3569 => l = 1, l = 10, ..., l = 1000)
        int64_t r = get_digit(n, 0);          // prefix of N                 (ex: n = 3569 => r = 3, r = 35, ..., r = 3569)
        dp[0] = 0;                            // dp[i] - ans for i-th prefix of N
        for (size_t i = 1; i <= r; ++i) dp[0] += (i % m == 0);
        for (size_t i = 1; i < log_10(n); ++i) {
            l *= 10;
            r = r * 10 + get_digit(n, i);
            dp[i] = dp[i - 1] + 1 + ((r - r % m) - (l + (m - l % m) % m)) / m;
        }
        cout << dp[log_10(n) - 1] - (n % m == 0) << '\n';
    }
    return 0;
}

