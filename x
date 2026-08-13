define gcd(a, b) {
    auto t
    while (b != 0) {
        t = a % b
        a = b
        b = t
    }
    return(a)
}

gcd(123456, 7890)
