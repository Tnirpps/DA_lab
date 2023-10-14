#include <iostream>
#include <vector>
#include <algorithm>

/*
https://www.csd.uwo.ca/~mmorenom/CS874/Lectures/Newton2Hensel.html/Newton2Hensel.html
https://bigprimes.org/
https://ei1333.github.io/algorithm/fft.html
https://www.nayuki.io/page/number-theoretic-transform-integer-dft
http://numbers.computation.free.fr/Constants/constants.html
https://ntnuopen.ntnu.no/ntnu-xmlui/bitstream/handle/11250/2778380/no.ntnu%3Ainspera%3A71444464%3A16855169.pdf?sequence=1
https://codeforces.com/contestInvitation/71eb9ccf6b5955e67fe280532eebb0fa46fad483
https://www.e-olymp.com/en/problems/1327

|n| <= 2e5
====================== Table ====================
mod                | primitive  root  | max digit
-------------------|------------------|----------
26214401           | 3                | 10
167772161          | 3                | 10
262146359297       | 3                | 1000
1310721835009      | 7                | 1000
2621444718593      | 3                | 1000
262144001310721    | 7                | 10000
2621440000000001   | 3                | 100000
262144000000524289 | 13               | 1000000
524288000000524289 | 3                | 1000000
*/

class TBigInt {
public:
    static const int64_t DIGIT = 1000000;
    static const int64_t DIGIT_COUNT = 6;
    static const int64_t BASE = 10;

    static const int64_t KARATSUBA_MUL_SIZE = 100;
    static const int64_t NTT_MUL_SIZE = 500;

    static const int64_t MOD = 524288000000524289;
    static const int64_t PRIMITIVE_ROOT = 3;
    
    TBigInt() = default;
    TBigInt(const size_t& size) : data(size) {}
    TBigInt(const TBigInt& num) : data(num.data) {}
    TBigInt(const std::string& s);
    TBigInt& operator = (const TBigInt & num) {
        data = num.data;
        return *this;
    }

    size_t size() const { return data.size(); }

    friend TBigInt operator + (const TBigInt& lhs, const TBigInt& rhs);
    friend TBigInt operator - (const TBigInt& lhs, const TBigInt& rhs);
    friend TBigInt operator * (const TBigInt& lhs, const TBigInt& rhs);
    friend TBigInt operator ^ (TBigInt lhs, TBigInt rhs);
    friend TBigInt operator / (const TBigInt& lhs, const TBigInt& rhs);
    friend TBigInt operator % (const TBigInt& lhs, const TBigInt& rhs);
    friend bool operator < (const TBigInt& lhs, const TBigInt& rhs);
    friend bool operator > (const TBigInt& lhs, const TBigInt& rhs);
    friend bool operator == (const TBigInt& lhs, const TBigInt& rhs);
    friend std::ostream & operator << (std::ostream & out, const TBigInt & num);
    friend std::istream & operator >> (std::istream & in, TBigInt & num);
    static void ntt(std::vector<int64_t> & v, bool inverse);

private:
    std::vector<int64_t> data;

    static int64_t mul(int64_t a, int64_t b) { return (int64_t)(((__int128)a * b) % MOD); }
    static int64_t add(int64_t a, int64_t b) { return (a + b) % MOD; }
    static int64_t dec(int64_t a, int64_t b) { return ((a - b) % MOD + MOD) % MOD; }
    static int64_t pow_mod(int64_t a, int64_t b);
    static int64_t roudn_to_pow_2(int64_t n);
    
    void normalize();
    static std::vector<int64_t> slow_mul(const std::vector<int64_t>& v1, const std::vector<int64_t>& v2);
    static std::vector<int64_t> fast_mul(const std::vector<int64_t>& v1, const std::vector<int64_t>& v2);
    static std::vector<int64_t> karatsuba_mul(const std::vector<int64_t>& v1, const std::vector<int64_t>& v2);
    static std::vector<int64_t> karatsubaMultiply(const std::vector<int64_t> &v1, const std::vector<int64_t> &v2);
    static void div_mod2(const TBigInt& num, TBigInt& div, int64_t & mod);
    static void div_mod(const TBigInt& lhs, const TBigInt& rhs, TBigInt& div, TBigInt& mod);
};

TBigInt::TBigInt(const std::string& s) {
    size_t n = s.size();
    data.resize((n + DIGIT_COUNT - 1) / DIGIT_COUNT);
    for (size_t i = 0; i < data.size(); ++i) {
        int64_t digit = 0;
        for (int64_t j = 0; j < TBigInt::DIGIT_COUNT; ++j) {
            int64_t ind = n - (i + 1) * TBigInt::DIGIT_COUNT + j;
            if (ind >= 0) {
                digit = digit * TBigInt::BASE + s[ind] - '0';
            }
        }
        data[data.size() - 1 - i] = digit;
    }
}

std::istream & operator >> (std::istream & in, TBigInt & num) {
    std::string s;
    in >> s;
    size_t i = 0;
    while (i < s.size() && s[i] == '0') {
        ++i;
    }
    if (i == s.size()) {
        num = TBigInt("0");
    } else {
        num = TBigInt(s.substr(i, s.size() - i + 1));
    }
    return in;
}

std::ostream & operator << (std::ostream & out, const TBigInt & num) {
    out << num.data[0];
    for (size_t i = 1; i < num.size(); ++i) {
        int64_t curDigit = TBigInt::DIGIT / TBigInt::BASE;
        while (curDigit >= TBigInt::BASE - 1 && num.data[i] / curDigit == 0) {
            out << '0';
            curDigit = curDigit / TBigInt::BASE;
        }
        out << num.data[i];
    }
    return out;
}

int64_t TBigInt::pow_mod(int64_t a, int64_t b) {
    int64_t s = 1;
    while (b) {
        if (b & 1)
            s = mul(s, a);
        a = mul(a, a);
        b >>= 1;
    }
    return s;
}

int64_t TBigInt::roudn_to_pow_2(int64_t n) {
    int64_t res = 1;
    while (res < n) {
        res *= 2;
    }
    return res;
}

bool operator == (const TBigInt& lhs, const TBigInt& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (size_t i = 0; i < lhs.size(); ++i) {
        if (lhs.data[i] != rhs.data[i]) {
            return false;
        }
    }
    return true;
}

bool operator < (const TBigInt& lhs, const TBigInt& rhs) {
    if (lhs.size() != rhs.size()) {
        return lhs.size() < rhs.size();
    }
    for (size_t i = 0; i < lhs.size(); ++i) {
        if (lhs.data[i] != rhs.data[i]) {
            return lhs.data[i] < rhs.data[i];
        }
    }
    return false;
}

bool operator > (const TBigInt& lhs, const TBigInt& rhs) {
    if (lhs.size() != rhs.size()) {
        return lhs.size() > rhs.size();
    }
    for (size_t i = 0; i < lhs.size(); ++i) {
        if (lhs.data[i] != rhs.data[i]) {
            return lhs.data[i] > rhs.data[i];
        }
    }
    return false;
}

TBigInt operator + (const TBigInt& lhs, const TBigInt& rhs) {
    if (lhs.size() < rhs.size()) {
        return rhs + lhs;
    }
    TBigInt res(lhs.size());
    bool flag = false;
    for (size_t i = 0; i < lhs.size(); ++i) {
        int64_t sum;
        if (i > rhs.size() - 1) {
            sum = lhs.data[lhs.size() - i - 1];
        } else {
            sum = lhs.data[lhs.size() - i - 1] + rhs.data[rhs.size() - i - 1];
        }
        if (flag) {
            ++sum;
        }
        if (sum >= TBigInt::DIGIT) {
            sum = sum - TBigInt::DIGIT;
            flag = true;
        } else {
            flag = false;
        }
        res.data[i] = sum;
    }
    if (flag) {
        res.data.push_back(1);
    }
    res.normalize();
    return res;
}

TBigInt operator - (const TBigInt& lhs, const TBigInt& rhs) {
    TBigInt res(lhs.size());
    bool flag = false;
    for (size_t i = 0; i < lhs.size(); ++i) {
        int64_t diff;
        if (i > rhs.size() - 1) {
            diff = lhs.data[lhs.size() - i - 1];
        } else {
            diff = lhs.data[lhs.size() - i - 1] - rhs.data[rhs.size() - i - 1];
        }
        if (flag) {
            --diff;
        }
        if (diff < 0) {
            diff = diff + TBigInt::DIGIT;
            flag = true;
        } else {
            flag = false;
        }
        res.data[i] = diff;
    }
    if (flag) {
        --res.data.back();
    }
    res.normalize();
    return res;
}

TBigInt operator * (const TBigInt& lhs, const TBigInt& rhs) {
    TBigInt res;
    if (std::min(lhs.size(), rhs.size()) < TBigInt::KARATSUBA_MUL_SIZE) {
        res.data = TBigInt::slow_mul(lhs.data, rhs.data);
    } else if (std::min(lhs.size(), rhs.size()) < TBigInt::NTT_MUL_SIZE) {
        res.data = TBigInt::karatsuba_mul(lhs.data, rhs.data);
    } else {
        res.data = TBigInt::fast_mul(lhs.data, rhs.data);
    }

    for (size_t i = 0; i + 1 < res.size(); ++i) {
        res.data[i + 1] = res.data[i + 1] + res.data[i] / TBigInt::DIGIT;
        res.data[i] = res.data[i] % TBigInt::DIGIT;
    }
    res.normalize();
    return res;
}


TBigInt operator ^ (TBigInt lhs, TBigInt rhs) {
    TBigInt res("1");
    while (rhs.size() > 0 && rhs.data[0] > 0) {
        TBigInt div;
        int64_t mod;
        TBigInt::div_mod2(rhs, div, mod);
        if (mod > 0) {
            res = res * lhs;
        }
        lhs = lhs * lhs;
        rhs = div;
    }
    return res;
}

TBigInt operator / (const TBigInt& lhs, const TBigInt& rhs) {
    TBigInt div, mod;
    TBigInt::div_mod(lhs, rhs, div, mod);
    return div;
}

TBigInt operator % (const TBigInt& lhs, const TBigInt& rhs) {
    TBigInt div, mod;
    TBigInt::div_mod(lhs, rhs, div, mod);
    return mod;
}

void TBigInt::div_mod2(const TBigInt& num, TBigInt& div, int64_t & mod) {
    TBigInt res;
    int64_t rem = 0;
    bool flag = true;
    for (size_t i = 0; i < num.size(); ++i) {
        rem = rem * DIGIT + num.data[i];
        if (rem < 2 && flag) {
            continue;
        }
        res.data.push_back(rem / 2);
        rem %= 2;
        flag = false;
    }
    if (res.data.empty()) {
        div = TBigInt(0);
    } else {
        div = res;
    }
    mod = rem;
}

// v.size() must be power of 2
void TBigInt::ntt(std::vector<int64_t> & v, bool inverse) {
    // butterfly
    const int N = v.size();
    for (int i = 0, j = 1; j + 1 < N; j++) {
        for (int k = (N >> 1); k > (i ^= k); k >>= 1);
        if (i > j) std::swap(v[i], v[j]);
    }
 
    int64_t w, wn, power, tmp;
    for (int i = 2; i <= N; i <<= 1) {
        if (inverse) {
            power = MOD - 1 - (MOD - 1) / i;
        } else {
            power = (MOD - 1) / i;
        }
        wn = pow_mod(PRIMITIVE_ROOT, power);
        for (int j = 0; j < N; j += i) {
            w = 1;
            for (int k = 0; k < (i / 2); k++) {
                tmp = mul(w, v[i / 2 + j + k]);
                v[i / 2 + j + k] = dec(v[j + k], tmp);
                v[j + k] = add(v[j + k], tmp);
                w = mul(w, wn);
            }
        }
    }
    if (inverse) {
        int64_t invert_size = pow_mod(N, MOD - 2);
        for (int i = 0; i < N; i++) v[i] = mul(v[i], invert_size);
    }
}

std::vector<int64_t> TBigInt::slow_mul(const std::vector<int64_t>& v1, const std::vector<int64_t>& v2) {
    std::vector<int64_t> res(v1.size() + v2.size());
    for (size_t i = 0; i < v1.size(); ++i) {
        for (size_t j = 0; j < v2.size(); ++j) {
            res[i + j] += v1[v1.size() - i - 1] * v2[v2.size() - j - 1];
        }
    }
    return res;
}

std::vector<int64_t> TBigInt::fast_mul(const std::vector<int64_t>& v1, const std::vector<int64_t>& v2) {
    size_t size = roudn_to_pow_2(v1.size() + v2.size());
    std::vector<int64_t> a(size), b(size);
    for (size_t i = 0; i < v1.size(); ++i) a[v1.size() - i - 1] = v1[i];
    for (size_t i = 0; i < v2.size(); ++i) b[v2.size() - i - 1] = v2[i];
    ntt(a, false);
    ntt(b, false);
    std::vector<int64_t> c(size);
    for (size_t i = 0; i < size; ++i) {
        c[i] = mul(a[i], b[i]);
    }
    ntt(c, true);
    return c;
}

std::vector<int64_t> TBigInt::karatsubaMultiply(const std::vector<int64_t> &a, const std::vector<int64_t> &b) {
    size_t n = a.size();
    std::vector<int64_t> res(n + n);
    if (n <= 32) {
        for (size_t i = 0; i < n; i++) {
            for (size_t j = 0; j < n; j++) {
                res[i + j] += a[i] * b[j];
            }
        }
        return res;
    }

    size_t k = n >> 1;
    std::vector<int64_t> a1(a.begin(), a.begin() + k);
    std::vector<int64_t> a2(a.begin() + k, a.end());
    std::vector<int64_t> b1(b.begin(), b.begin() + k);
    std::vector<int64_t> b2(b.begin() + k, b.end());

    std::vector<int64_t> a1b1 = karatsubaMultiply(a1, b1);
    std::vector<int64_t> a2b2 = karatsubaMultiply(a2, b2);

    for (size_t i = 0; i < k; i++) a2[i] += a1[i];
    for (size_t i = 0; i < k; i++) b2[i] += b1[i];

    std::vector<int64_t> r = karatsubaMultiply(a2, b2);
    for (size_t i = 0; i < a1b1.size(); i++) r[i] -= a1b1[i];
    for (size_t i = 0; i < a2b2.size(); i++) r[i] -= a2b2[i];

    for (size_t i = 0; i < r.size(); i++) res[i + k] += r[i];
    for (size_t i = 0; i < a1b1.size(); i++) res[i] += a1b1[i];
    for (size_t i = 0; i < a2b2.size(); i++) res[i + n] += a2b2[i];
    return res;
}

std::vector<int64_t> TBigInt::karatsuba_mul(const std::vector<int64_t>& v1, const std::vector<int64_t>& v2) {
    int64_t n = TBigInt::roudn_to_pow_2(std::max(v1.size(), v2.size()));
    std::vector<int64_t> x(n);
    std::vector<int64_t> y(n);
    std::copy(v1.rbegin(), v1.rend(), x.begin());
    std::copy(v2.rbegin(), v2.rend(), y.begin());
    return karatsubaMultiply(x, y);
}

void TBigInt::normalize() {
    while (data.size() > 1 && data.back() == 0) {
        data.pop_back();
    }
    std::reverse(data.begin(), data.end());
}

void TBigInt::div_mod(const TBigInt& lhs, const TBigInt& rhs, TBigInt& div, TBigInt& mod) {
    if (lhs.size() < rhs.size()) {
        div = TBigInt("0");
        mod = lhs;
        return;
    }
    TBigInt res(lhs.size());
    TBigInt tmp(rhs.size());
    tmp.data.back() = lhs.data[0];
    TBigInt shiftFactor(2);
    shiftFactor.data[0] = 1;
    for (size_t i = 0; i < lhs.size(); ++i) {
        int64_t l = -1;
        int64_t r = TBigInt::DIGIT;
        TBigInt factor(1);
        while (l + 1 < r) {
            int64_t m = (l + r) / 2;
            factor.data[0] = m;
            if (factor * rhs > tmp) {
                r = m;
            } else {
                l = m;
            }
        }
        res.data[res.size() - i - 1] = l;
        factor.data[0] = l;
        tmp = (tmp - factor * rhs) * shiftFactor;
        if (i + 1 < lhs.size()) {
            tmp.data.back() = lhs.data[i + 1];
        }
    }
    res.normalize();
    div = res;
    mod = tmp;
}


const TBigInt BIG_ZERO = TBigInt("0");

int main() {
    std::ios_base::sync_with_stdio(0);
    std::cout.tie(nullptr);
    std::cin.tie(nullptr);
 
    TBigInt num1, num2;
    char action;
    std::cout << std::boolalpha;
    while (std::cin >> num1 >> num2 >> action) {
        if (action == '+') {
            std::cout << num1 + num2 << '\n';
        }
        if (action == '-') {
            if (num1 < num2) {
                std::cout << "Error\n";
            } else {
                std::cout << num1 - num2 << '\n';
            }
        }
        if (action == '*') {
            std::cout << num1 * num2 << '\n';
        }
        if (action == '^') {
            if (num1 == BIG_ZERO && num2 == BIG_ZERO) {
                std::cout << "Error\n";
            } else {
                std::cout << (num1 ^ num2) << '\n';
            }
        }
        if (action == '/') {
            if (num2 == BIG_ZERO) {
                std::cout << "Error\n";
            } else {
                std::cout << num1 / num2 << '\n';
            }
        }
        if (action == '<') {
            std::cout << (num1 < num2) << '\n';
        }
        if (action == '>') {
            std::cout << (num1 > num2) << '\n';
        }
        if (action == '=') {
            std::cout << (num1 == num2) << '\n';
        }
    }
    return 0;
}

