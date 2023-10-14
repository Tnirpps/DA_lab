#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

using namespace std;

int main() {
    ios_base::sync_with_stdio(0);
    cout.tie(nullptr);
    cin.tie(nullptr);
    int n;
    cin >> n;
    vector<int64_t> dp(n + 1);
    vector<size_t> action(n + 1);
    for (size_t i = 2; i <= n; ++i) {
        vector<int64_t> choise(3, dp[i - 1]);
        if (i % 2 == 0) choise[1] = dp[i / 2];
        if (i % 3 == 0) choise[2] = dp[i / 3];
        action[i] = distance(choise.begin(), min_element(choise.begin(), choise.end()));
        dp[i] = i + dp[i / (action[i] + 1) - (action[i] == 0)];
    }
    cout << dp[n] << '\n';
    string ans, act_str[3] = {"-1 ", "/2 ", "/3 "};
    while (n > 1) {
        cout << act_str[action[n]];
        n = n / (action[n] + 1) - (action[n] == 0);
    }
    cout << '\n';
    return 0;
}

