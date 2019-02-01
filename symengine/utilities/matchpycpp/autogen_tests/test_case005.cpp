/*
 * This file was automatically generated: DO NOT EDIT.
 *
 * Decision tree matching expressions ['x**w']
 */
#include "catch.hpp"
#include <deque>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <symengine/basic.h>
#include <symengine/pow.h>
#include <symengine/utilities/matchpycpp/common.h>
#include <symengine/utilities/matchpycpp/substitution.h>
#include <tuple>

generator<tuple<int, Substitution>> match_root(RCP<const Basic> subject)
{
    generator<tuple<int, Substitution>> result;
    Deque subjects;
    subjects.push_front(subject);
    Substitution subst0;
    // State 2217
    if (subjects.size() >= 1 && is_a<Pow>(*subjects[0])) {
        RCP<const Basic> tmp1 = subjects.front();
        subjects.pop_front();
        Deque subjects2 = get_deque(tmp1);
        // State 2218
        if (subjects2.size() >= 1 && subjects2[0]->__eq__(*x)) {
            RCP<const Basic> tmp3 = subjects2.front();
            subjects2.pop_front();
            // State 2219
            if (subjects2.size() >= 1) {
                RCP<const Basic> tmp4 = subjects2.front();
                subjects2.pop_front();
                Substitution subst1 = Substitution(subst0);
                if (!try_add_variable(subst1, "i2", tmp4)) {
                    // State 2220
                    if (subjects2.size() == 0) {
                        // State 2221
                        if (subjects.size() == 0) {
                            Substitution tmp_subst;
                            tmp_subst["w"] = subst1["i2"];
                            // 0: x**w
                            result.push_back(make_tuple(0, tmp_subst));
                        }
                    }
                }
                subjects2.push_front(tmp4);
            }
            subjects2.push_front(tmp3);
        }
        subjects.push_front(tmp1);
    }
    return result;
}

TEST_CASE("GeneratedMatchPyTest4", "")
{
    generator<tuple<int, Substitution>> ret;
    Substitution substitution;

    // Pattern x + y not matching:
    ret = match_root(add(x, y));
    REQUIRE(ret.size() == 0);

    // Pattern x**w matching x**2 with substitution {}:
    ret = match_root(pow(x, integer(2)));
    substitution = get<1>(ret[0]);
    REQUIRE(ret.size() > 0);
    REQUIRE(get<0>(ret[0]) == 0);

    // Pattern x**w matching x**3 with substitution {}:
    ret = match_root(pow(x, integer(3)));
    substitution = get<1>(ret[0]);
    REQUIRE(ret.size() > 0);
    REQUIRE(get<0>(ret[0]) == 0);
}
