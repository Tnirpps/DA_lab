#include <iostream>
#include <vector>

using namespace std;

int main() {
    string s;
    cin >> s;
    size_t n = s.size();
    vector<vector<int64_t>> dp (n, vector<int64_t>(n));
    for (size_t i = 0; i < n; ++i) dp[i][i] = 1;
    for (size_t len = 1; len < n; ++len) {
        for (size_t i = 0; i + len < n; ++i) {
            size_t j = i + len;
            if (s[i] == s[j]) {
                 dp[i][j] = dp[i + 1][j] + dp[i][j - 1] + 1;
            } else {
                dp[i][j] = dp[i + 1][j] + dp[i][j - 1] - dp[i + 1][j - 1];
            }
        }
    }
    cout << dp[0][n - 1] << "\n";
    return 0;
}

