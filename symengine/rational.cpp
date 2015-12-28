#include <symengine/rational.h>
#include <symengine/pow.h>
#include <symengine/mul.h>

namespace SymEngine {

Rational::Rational(rational_class i)
    : i{i}
{
    SYMENGINE_ASSERT(is_canonical(this->i))
}

bool Rational::is_canonical(const rational_class &i) const
{
    rational_class x = i;
    canonicalize(x);
    // If 'x' is an integer, it should not be Rational:
    if (SymEngine::get_den(x) == 1) return false;
    // if 'i' is not in canonical form:
    if (SymEngine::get_num(x) != SymEngine::get_num(i)) return false;
    if (SymEngine::get_den(x) != SymEngine::get_den(i)) return false;
    return true;
}

RCP<const Number> Rational::from_mpq(rational_class i)
{
    // If the result is an Integer, return an Integer:
    if (SymEngine::get_den(i) == 1) {
        return integer(SymEngine::get_num(i));
    } else {
        return make_rcp<const Rational>(i);
    }
}

RCP<const Number> Rational::from_two_ints(const Integer &n,
            const Integer &d)
{
    if (d.i == 0)
        throw std::runtime_error("Rational: Division by zero.");
    rational_class q(n.i, d.i);

    // This is potentially slow, but has to be done, since 'n/d' might not be
    // in canonical form.
    canonicalize(q);

    return Rational::from_mpq(std::move(q));
}

RCP<const Number> Rational::from_two_ints(long n, long d)
{
    if (d == 0)
        throw std::runtime_error("Rational: Division by zero.");
    rational_class q(n, d);

    // This is potentially slow, but has to be done, since 'n/d' might not be
    // in canonical form.
    canonicalize(q);

    return Rational::from_mpq(q);
}

std::size_t Rational::__hash__() const
{
    // only the least significant bits that fit into "signed long int" are
    // hashed:
    std::size_t seed = RATIONAL;
    hash_combine<long long int>(seed, get_si(SymEngine::get_num(this->i)));
    hash_combine<long long int>(seed, get_si(SymEngine::get_den(this->i)));
    return seed;
}

bool Rational::__eq__(const Basic &o) const
{
    if (is_a<Rational>(o)) {
        const Rational &s = static_cast<const Rational &>(o);
        return this->i == s.i;
    }
    return false;
}

int Rational::compare(const Basic &o) const
{
    if (is_a<Rational>(o)) {
        const Rational &s = static_cast<const Rational &>(o);
        if (i == s.i) return 0;
        return i < s.i ? -1 : 1;
    }
    if (is_a<Integer>(o)) {
        const Integer &s = static_cast<const Integer &>(o);
        return i < s.i ? -1 : 1;
    }
    throw std::runtime_error("unhandled comparison of Rational");
}

void get_num_den(const Rational &rat,
            const Ptr<RCP<const Integer>> &num,
            const Ptr<RCP<const Integer>> &den)
{
    *num = integer(SymEngine::get_num(rat.i));
    *den = integer(SymEngine::get_den(rat.i));
}

bool Rational::is_perfect_power(bool is_expected) const
{
    const integer_class &num = SymEngine::get_num(i);
    if (num == 0)
        return true;
    else if (num == 1)
        return mpz_perfect_power_p(get_mpz_t(SymEngine::get_den(i))) != 0;

    const integer_class &den = SymEngine::get_den(i);

    if (not is_expected) {
        if (mpz_cmpabs(get_mpz_t(num), get_mpz_t(den)) > 0) {
            if (mpz_perfect_power_p(get_mpz_t(den)) == 0)
                return false;
        }
        else {
            if (mpz_perfect_power_p(get_mpz_t(num)) == 0)
                return false;
        }
    }
    integer_class prod = num * den;
    return mpz_perfect_power_p(get_mpz_t(prod)) != 0;
}

bool Rational::nth_root(const Ptr<RCP<const Number>> &the_rat, unsigned long n) const
{
    if (n == 0)
        throw std::runtime_error("i_nth_root: Can not find Zeroth root");

    rational_class r;
    int ret = mpz_root(get_mpz_t(SymEngine::get_num(r)), get_mpz_t(SymEngine::get_num(i)), n);
    if (ret == 0)
        return false;
    ret = mpz_root(get_mpz_t(SymEngine::get_den(r)), get_mpz_t(SymEngine::get_den(i)), n);
    if (ret == 0)
        return false;
    // No need to canonicalize since `this` is in canonical form
    *the_rat = make_rcp<const Rational>(r);
    return true;
}

RCP<const Basic> Rational::powrat(const Rational &other) const {
    return SymEngine::mul(other.rpowrat(*this->get_num()), other.neg()->rpowrat(*this->get_den()));
}

RCP<const Basic> Rational::rpowrat(const Integer &other) const {
    if (not (fits_ulong_p(SymEngine::get_den(i))))
        throw std::runtime_error("powrat: den of 'exp' does not fit ulong.");
    unsigned long exp = get_ui(SymEngine::get_den(i));
    RCP<const Integer> res;
    if (other.is_negative()) {
        if (i_nth_root(outArg(res), *other.neg(), exp)) {
            if (exp % 2 == 0) {
                return I->pow(*get_num())->mul(*res->powint(*get_num()));
            } else {
                return SymEngine::neg(res->powint(*get_num()));
            }
        }
    } else {
        if (i_nth_root(outArg(res), other, exp)) {
            return res->powint(*get_num());
        }
    }
    integer_class q, r;
    auto num = SymEngine::get_num(i);
    auto den = SymEngine::get_den(i);

    mpz_fdiv_qr(get_mpz_t(q), get_mpz_t(r), get_mpz_t(num),
                get_mpz_t(den));
    // Here we make the exponent postive and a fraction between
    // 0 and 1. We multiply numerator and denominator appropriately
    // to achieve this
    RCP<const Number> coef = other.powint(*integer(q));
    map_basic_basic surd;

    if ((other.is_negative()) and den == 2) {
        imulnum(outArg(coef), I);
        // if other.neg() is one, no need to add it to dict
        if (other.i != -1)
            insert(surd, other.neg(), Rational::from_mpq(rational_class(r, den)));
    } else {
        insert(surd, other.rcp_from_this(), Rational::from_mpq(rational_class(r, den)));
    }
    return Mul::from_dict(coef, std::move(surd));
}

} // SymEngine
