#include <iostream>

int main () {
    size_t n, p, m;
    std::cin >> n >> p >> m;
    for (size_t i = 0; i + 1 < n; i++)  {
        std::cout << m % p << '\n';
        m /= p;
    }
    std::cout << ((m > 0) ? m : 0) << '\n';
    return 0;
}

