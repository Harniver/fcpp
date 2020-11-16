// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file plot.hpp
 * @brief Plot building tools.
 */

#ifndef FCPP_COMMON_PLOT_H_
#define FCPP_COMMON_PLOT_H_

#include <cstdint>
#include <limits>
#include <map>
#include <ostream>
#include <ratio>
#include <set>
#include <sstream>
#include <vector>

#include "lib/common/mutex.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/option/aggregator.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Namespace for filtering functions.
namespace filter {
    //! @brief Filters values within `L/den` and `U/den` (included).
    template <intmax_t L, intmax_t U, intmax_t den = 1>
    struct within {
        template <typename V>
        bool operator()(V v) const {
            v *= den;
            if (L > std::numeric_limits<intmax_t>::min() and L > v) return false;
            if (U < std::numeric_limits<intmax_t>::max() and U < v) return false;
            return true;
        }
    };

    //! @brief Filters values above `L/den` (included).
    template <intmax_t L, intmax_t den = 1>
    using above = within<L, std::numeric_limits<intmax_t>::max(), den>;

    //! @brief Filters values below `U/den` (included).
    template <intmax_t U, intmax_t den = 1>
    using below = within<std::numeric_limits<intmax_t>::min(), U, den>;

    //! @brief Filters values equal to `V/den`.
    template <intmax_t V, intmax_t den = 1>
    using equal = within<V, V, den>;
}


//! @brief Namespace for plot building tools.
namespace plot {

//! @brief Tag for time information.
struct time {};

//! @cond INTERNAL
namespace details {
    //! @brief Check if character is a vowel.
    bool isvowel(char c) {
        std::string vowels = "aeiouAEIOU";
        for (char v : vowels) if (c == v) return true;
        return false;
    }
    //! @brief Check if character is alphabetic.
    bool isalpha(char c) {
        if ('a' <= c and c <= 'z') return true;
        if ('A' <= c and c <= 'Z') return true;
        return false;
    }
    //! @brief Shortens a string.
    std::string shorten(std::string s) {
        std::string pre;
        std::string main;
        while (not isalpha(s.back())) s.pop_back();
        for (size_t i=0; i<s.size(); ++i) {
            if (isalpha(s[i])) {
                if (main.size() < 3 or not isvowel(s[i])) main.push_back(s[i]);
            } else {
                if (main.size()) pre.push_back(main[0]);
                main = "";
            }
        }
        return pre + main;
    }
    //! @brief Shortens a title string.
    std::string multi_shorten(std::string s) {
        if (s.empty()) return s;
        std::vector<std::string> tags;
        std::vector<std::string> vals;
        size_t pos = 0, np;
        while (true) {
            np = s.find(" = ", pos);
            tags.push_back(shorten(s.substr(pos, np-pos)));
            pos = np+3;
            np = s.find(", ", pos);
            vals.push_back(s.substr(pos, np-pos));
            if (np == std::string::npos) break;
            pos = np+2;
        }
        s = "";
        for (size_t i=0; i<tags.size(); ++i)
            s += tags[i] + vals[i];
        return s;
    }
}
//! @endcond

//! @brief Structure representing a point in a plot.
struct point {
    //! @brief Default constructor.
    point() = default;
    //! @brief The unit of the measured value.
    std::string unit;
    //! @brief The source of the measured value.
    std::string source;
    //! @brief The measured value.
    double value;
};

//! @brief Printing a single page.
std::ostream& operator<<(std::ostream& o, point const& p) {
    o << "(";
    if (p.unit.size()) o << p.unit << ", ";
    return o << p.source << ", " << p.value << ")";
}

//! @brief Structure representing a single plot.
struct plot {
    //! @brief Default constructor.
    plot() = default;
    //! @brief Title of the plot.
    std::string title;
    //! @brief Label of the x axis.
    std::string xname;
    //! @brief Label of the y axis.
    std::string yname;
    //! @brief Values of x coordinates.
    std::vector<double> xvals;
    //! @brief Values of y coordinates with labels.
    std::vector<std::pair<std::string, std::vector<double>>> yvals;
};

//! @brief Printing a single plot.
std::ostream& operator<<(std::ostream& o, plot const& p) {
    o << "plot.put(plot.plot(name+\"-" << details::shorten(p.xname) << details::shorten(p.yname) << (p.title.size() ? "-" : "") << details::multi_shorten(p.title) << "\", \"" << p.title << "\", \"" << p.xname << "\", \"" << p.yname << "\", new string[] {";
    bool first = true;
    for (auto const& y : p.yvals) {
        if (first) first = false;
        else o << ", ";
        o << "\"" << y.first << "\"";
    }
    o << "}, new pair[][] {";
    first = true;
    for (auto const& y : p.yvals) {
        if (first) first = false;
        else o << ", ";
        o << "{";
        for (size_t i=0; i<y.second.size(); ++i) {
            if (i > 0) o << ", ";
            o << "(" << p.xvals[i] << ", " << y.second[i] << ")";
        }
        o << "}";
    }
    return o << "}));\n";
}

//! @brief Structure representing a page of plots.
struct page {
    //! @brief Default constructor.
    page() = default;
    //! @brief Constructor with a vector of plots.
    page(std::vector<plot> p) : title(), plots(p) {
        rows = 1;
        cols = plots.size();
    }
    //! @brief Constructor with an array of plots.
    template <size_t N>
    page(std::array<plot, N> p) : title(), plots(p.begin(), p.end()) {
        rows = 1;
        cols = plots.size();
    }
    //! @brief Title of the page.
    std::string title;
    //! @brief Number of rows.
    size_t rows;
    //! @brief Number of columns.
    size_t cols;
    //! @brief Plot list.
    std::vector<plot> plots;
};

//! @brief Printing a single page.
std::ostream& operator<<(std::ostream& o, page const& p) {
    if (p.title.size()) o << "// " << p.title << "\n\n";
    o << "plot.ROWS = " << p.rows << ";\n";
    o << "plot.COLS = " << p.cols << ";\n\n";
    for (plot const& q : p.plots) o << q << "\n";
    return o;
}

//! @brief Structure representing a whole file of plots.
struct file {
    //! @brief Constructor with a vector of pages.
    file(std::string title, std::vector<page> p) : title(title), pages(p) {}
    //! @brief Constructor with an array of pages.
    template <size_t N>
    file(std::string title, std::array<page, N> p) : title(title), pages(p.begin(), p.end()) {}
    //! @brief Constructor with a vector of plots.
    file(std::string title, std::vector<plot> p) : title(title) {
        pages.emplace_back(p);
    }
    //! @brief Constructor with an array of plots.
    template <size_t N>
    file(std::string title, std::array<plot, N> p) : title(title) {
        pages.emplace_back(p);
    }
    //! @brief Title of the file.
    std::string title;
    //! @brief Page list.
    std::vector<page> pages;
};

//! @brief Printing a file.
std::ostream& operator<<(std::ostream& o, file const& f) {
    o << "// " << f.title << "\n";
    o << "string name = \"" << f.title << "\";\n\n";
    o << "import \"plot.asy\" as plot;\n";
    o << "unitsize(1cm);\n\n";
    for (page const& p : f.pages) o << p << "\n";
    o << "shipout(\"" << f.title << "\");\n";
    return o;
}


//! @brief Empty class not producing a plot.
class none {
  public:
    //! @brief The internal build type.
    using build_type = typename std::array<point, 0>;

    //! @brief Default constructor.
    none() = default;

    //! @brief Row processing.
    template <typename R>
    none& operator<<(R const&) {
        return *this;
    }

    //! @brief Plot building for internal use.
    build_type build() const {
        return {};
    }
};


//! @brief Filters values for column S according to property F in plotter P.
template <typename S, typename F, typename P>
class filter {
  public:
    //! @brief The internal build type.
    using build_type = typename P::build_type;

    //! @brief Default constructor.
    filter() = default;

    //! @brief Row processing.
    template <typename R>
    filter& operator<<(R const& row) {
        if (m_filter(common::get<S>(row)))
            (m_plotter << row);
        return *this;
    }

    //! @brief Plot building for internal use.
    build_type build() const {
        return m_plotter.build();
    }

  private:
    //! @brief The plotter.
    P m_plotter;
    //! @brief The callable filter class.
    F m_filter;
};


//! @cond INTERNAL
namespace details {
    //! @brief Concatenate vector and array types.
    template <typename... Ts>
    struct array_cat;
    //! @brief Concatenate one vector type.
    template <typename T>
    struct array_cat<std::vector<T>> {
        using type = std::vector<T>;
    };
    //! @brief Concatenate one array type.
    template <typename T, size_t N>
    struct array_cat<std::array<T,N>> {
        using type = std::array<T,N>;
    };
    //! @brief Concatenate two compatible vector types.
    template <typename T>
    struct array_cat<std::vector<T>, std::vector<T>> {
        using type = std::vector<T>;
    };
    //! @brief Concatenate vector and array types.
    template <typename T, size_t N>
    struct array_cat<std::vector<T>, std::array<T,N>> {
        using type = std::vector<T>;
    };
    //! @brief Concatenate array and vector types.
    template <typename T, size_t N>
    struct array_cat<std::array<T,N>, std::vector<T>> {
        using type = std::vector<T>;
    };
    //! @brief Concatenate two compatible array types.
    template <typename T, size_t N, size_t M>
    struct array_cat<std::array<T,N>, std::array<T,M>> {
        using type = std::array<T,N+M>;
    };
    //! @brief Concatenate multiple vector and array types.
    template <typename T, typename U, typename... Us>
    struct array_cat<T, U, Us...> {
        using type = typename array_cat<T, typename array_cat<U, Us...>::type>::type;
    };
    //! @brief Resizes a vector.
    template <typename T>
    void maybe_resize(std::vector<T>& v, size_t s) {
        v.resize(s);
    }
    //! @brief Does not resize an array.
    template <typename T, size_t N>
    void maybe_resize(std::array<T,N>&, size_t) {}
}
//! @endcond

//! @brief Joins a sequence of plotters.
template <typename... Ps>
class join {
  public:
    //! @brief The internal build type.
    using build_type = typename details::array_cat<typename Ps::build_type...>::type;

    //! @brief Default constructor.
    join() = default;

    //! @brief Row processing.
    template <typename R>
    join& operator<<(R const& row) {
        input_impl(row, std::make_index_sequence<sizeof...(Ps)>{});
        return *this;
    }

    //! @brief Plot building for internal use.
    build_type build() const {
        build_type res;
        build_impl(res, std::make_index_sequence<sizeof...(Ps)>{});
        return res;
    }

  private:
    //! @brief Forwarding rows to plotters.
    template <typename R, size_t... is>
    void input_impl(R const& row, std::index_sequence<is...>) {
        common::details::ignore((std::get<is>(m_plotters) << row)...);
    }

    //! @brief Stop joining plotter builds.
    void build_impl(build_type&, std::index_sequence<>, size_t) const {}

    //! @brief Joining plotter builds.
    template <size_t i, size_t... is>
    void build_impl(build_type& res, std::index_sequence<i, is...>, size_t x = 0) const {
        auto ri = std::get<i>(m_plotters).build();
        details::maybe_resize(res, x+ri.size());
        for (size_t j = 0; j < ri.size(); ++j)
            res[x+j] = std::move(ri[j]);
        build_impl(res, std::index_sequence<is...>{}, x + ri.size());
    }

    //! @brief The plotters.
    std::tuple<Ps...> m_plotters;
};

//! @brief Optimised Join of a single plotter.
template <typename P>
class join<P> : public P {};


//! @cond INTERNAL
namespace details {
    //! @brief Checks whether C has a static name method.
    template<typename C>
    struct has_name_method {
      private:
        template <typename T>
        static constexpr auto check(T*) -> typename std::is_same<
            decltype(T::name()),
            std::string
        >::type;

        template <typename>
        static constexpr std::false_type check(...);

        typedef decltype(check<C>(0)) type;

      public:
        static constexpr bool value = type::value;
    };
}
//! @endcond

//! @brief Maintains a value for the column S aggregated with A.
template <typename S, typename A = aggregator::mean<double>>
class value {
  public:
    //! @brief The internal build type.
    using build_type = std::array<point, 1>;

    //! @brief Default constructor.
    value() = default;

    //! @brief Row processing.
    template <typename R>
    value& operator<<(R const& row) {
        common::lock_guard<true> l(m_mutex);
        m_aggregator.insert(common::get<S>(row));
        return *this;
    }

    //! @brief Plot building for internal use.
    build_type build() const {
        point p;
        std::string t = tag_name(common::bool_pack<details::has_name_method<S>::value>{}); // tag name
        size_t pos = t.find("<");
        if (pos != std::string::npos) {
            p.unit = common::details::strip_namespaces(t.substr(0, pos));
            p.source = common::details::strip_namespaces(t.substr(pos+1, t.size()-pos-2));
        } else {
            pos = t.find("__");
            if (pos != std::string::npos) {
                p.source = common::details::strip_namespaces(t.substr(0, pos));
                p.unit = t.substr(pos+2);
            } else {
                p.unit = "";
                p.source = common::details::strip_namespaces(t);
            }
        }
        std::string ar = A::name(); // row aggregator
        std::string ad = aggregator_name(common::bool_pack<details::has_name_method<S>::value>{}); // device aggregator
        p.source += " (" + ad + ar + ")";
        p.value = std::get<0>(m_aggregator.template result<S>());
        return {p};
    }

  private:
    //! @brief Device aggregator name (if present).
    std::string aggregator_name(common::bool_pack<true>) const {
        return S::name() + "-";
    }
    //! @brief Device aggregator name (if absent).
    std::string aggregator_name(common::bool_pack<false>) const {
        return "";
    }

    //! @brief Tag name (if aggregator present).
    std::string tag_name(common::bool_pack<true>) const {
        return common::type_name<typename S::type>();
    }
    //! @brief Tag name (if aggregator absent).
    std::string tag_name(common::bool_pack<false>) const {
        return common::type_name<S>();
    }

    //! @brief A mutex for synchronised access to the aggregator.
    common::mutex<true> m_mutex;
    //! @brief The aggregator.
    A m_aggregator;
};


//! @brief Tag for declaring units to be extracted.
template <template<class> class... Ts>
struct unit;

//! @cond INTERNAL
namespace details {
    //! @brief Smart append of a plotter to a join.
    template <typename Q, typename... Ps>
    struct appender {
        using type = join<Ps..., Q>;
    };

    //! @brief Smart append of a join to a join.
    template <typename... Qs, typename... Ps>
    struct appender<join<Qs...>, Ps...> {
        using type = join<Ps..., Qs...>;
    };


    //! @brief Smart join of plotters.
    template <typename... Ps>
    struct joiner;

    //! @brief Smart join of one plotter.
    template <typename P>
    struct joiner<P> {
        using type = P;
    };

    //! @brief Smart join of two plotters (first non-join).
    template <typename P, typename Q>
    struct joiner<P, Q> : public appender<Q, P> {};

    //! @brief Smart join of two plotter (first join).
    template <typename... Ps, typename Q>
    struct joiner<join<Ps...>, Q> : public appender<Q, Ps...> {};

    //! @brief Smart join of multiple plotters.
    template <typename P, typename Q, typename... Qs>
    struct joiner<P, Q, Qs...>  : public joiner<typename joiner<P, Q>::type, Qs...> {};


    //! @brief Searches type T within tags and aggregators in S (general form).
    template <typename T, typename S>
    struct field_grep;

    //! @brief Searches type T within tags and aggregators in S.
    template <typename T, typename... Ss, typename... As>
    struct field_grep<T, common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<As...>>> {
        using type = common::type_cat<std::conditional_t<std::is_same<Ss,T>::value or std::is_same<As,T>::value, typename As::template result_type<Ss>::tags, common::type_sequence<>>...>;
    };


    //! @brief Searches template T within tags in S (general form).
    template <template<class> class T, typename S>
    struct unit_grep {
        using type = common::type_sequence<bool>;
    };

//! @brief Searches template T within tags in S.
    template <template<class> class T, typename... Ss, typename... As>
    struct unit_grep<T, common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<As...>>> {
        using type = common::type_cat<std::conditional_t<common::is_class_template<T,Ss>, typename As::template result_type<Ss>::tags, common::type_sequence<>>...>;
    };


    //! @brief Maintains values for multiple columns and aggregators (general form).
    template <typename S, typename A, typename... Ts>
    struct values;

    //! @brief Maintains values for one explicit column and one aggregator.
    template <template<class...> class S, typename S1, template<class...> class A, typename A1>
    struct values<S<S1>, A<A1>> {
       using type = value<S1, A1>;
    };

    //! @brief Maintains values for multiple explicit columns and one aggregator.
    template <template<class...> class S, typename S1, typename... Ss, template<class...> class A, typename A1>
    struct values<S<S1, Ss...>, A<A1>> {
       using type = join<value<S1, A1>, value<Ss, A1>...>;
    };

    //! @brief Maintains values for multiple explicit columns and no aggregators (defaults to `mean<double>`).
    template <typename S, template<class...> class A>
    struct values<S, A<>> : public values<S, A<aggregator::mean<double>>> {};

    //! @brief Maintains values for multiple explicit columns and multiple aggregators.
    template <typename S, template<class...> class A, typename A1, typename... As>
    struct values<S, A<A1, As...>> : public joiner<typename values<S, A<A1>>::type, typename values<S, A<As>>::type...> {};

    //! @brief Maintains values for multiple columns and aggregators, defined through a single field.
    template <template<class...> class S, typename... Ss, typename A, typename T>
    struct values<S<Ss...>, A, T> : public values<typename field_grep<T, common::tagged_tuple_t<common::type_sequence<Ss...>>>::type, A> {};

    //! @brief Maintains values for multiple columns and aggregators, defined through a single unit.
    template <template<class...> class S, typename... Ss, typename A, template<class> class T>
    struct values<S<Ss...>, A, unit<T>> : public values<typename unit_grep<T, common::tagged_tuple_t<common::type_sequence<Ss...>>>::type, A> {};

    //! @brief Maintains values for multiple columns and aggregators, defined through multiple units.
    template <typename S, typename A, template<class> class T, template<class> class... Ts>
    struct values<S, A, unit<T, Ts...>> : public joiner<typename values<S, A, unit<T>>::type, typename values<S, A, unit<Ts>>::type...> {};

    //! @brief Maintains values for multiple columns and aggregators, defined through multiple fields.
    template <typename S, typename A, typename T1, typename T2, typename... Ts>
    struct values<S, A, T1, T2, Ts...> : public joiner<typename values<S, A, T1>::type, typename values<S, A, T2>::type, typename values<S, A, Ts>::type...> {};
}
//! @endcond

/**
 * @brief Maintains values for multiple columns and aggregators.
 *
 * @param S The sequence of tags and aggregators for logging (intertwined).
 * @param A The sequence of row aggregators (if empty, `mean<double>` is assumed).
 * @param Ts Description of fields to be extracted as tags, aggregators or units (if empty, S is interpreted as fields).
 */
template <typename S, typename A, typename... Ts>
using values = typename details::values<S, A, Ts...>::type;


namespace details {
    //! @brief The type to be used for a single key.
    template <typename S>
    struct key_type {
        using type = common::tagged_tuple_t<S, double>;
        static constexpr bool single = true;
    };
    //! @brief The type to be used for multiple keys.
    template <typename... Ss>
    struct key_type<common::type_sequence<Ss...>> {
        using type = common::tagged_tuple_cat<common::tagged_tuple_t<Ss, double>...>;
        static constexpr bool single = (sizeof...(Ss) == 1);
    };
    //! @brief Inspects a build type.
    template <typename T>
    struct inspector;
    //! @brief Whether a vector build type contains a single object.
    template <typename T>
    struct inspector<std::vector<T>> {
        using type = T;
        static constexpr bool single = false;
    };
    //! @brief Whether an array build type contains a single object.
    template <typename T, size_t N>
    struct inspector<std::array<T,N>> {
        using type = T;
        static constexpr bool single = (N == 1);
    };
    //! @brief Promotes a maximal build type after split.
    template <typename P, bool single>
    struct promote_impl {
        using type = std::vector<page>;
    };
    //! @brief Promotes multiple plots after split.
    template <>
    struct promote_impl<plot, false> {
        using type = std::array<page, 1>;
    };
    //! @brief Promotes a single plot after split.
    template <>
    struct promote_impl<plot, true> {
        using type = std::vector<plot>;
    };
    //! @brief Promotes points after split.
    template <bool single>
    struct promote_impl<point, single> {
        using type = std::array<plot, 1>;
    };
    //! @brief Promotes a build type after split.
    template <typename P>
    using promote = typename promote_impl<typename inspector<P>::type, inspector<P>::single>::type;
}

/**
 * Split rows depending on
 *
 * @param S A column tag, or a `type_sequence` of column tags.
 * @param P The plotter to be split.
 * @param B A bucket size (as `std::ratio`, only for a single key).
 */
template <typename S, typename P, typename B = std::ratio<0>>
class split {
    //! @brief The build type of dependent plotters.
    using parent_build_type = typename P::build_type;

    static_assert(details::key_type<S>::single or not std::is_same<typename details::inspector<parent_build_type>::type, point>::value, "cannot split points with multiple keys");

    static_assert(details::key_type<S>::single or std::is_same<std::ratio<0>, B>::value, "cannot use bucket size with multiple keys");

    //! @brief The type used for splitting keys.
    using key_type = typename details::key_type<S>::type;

  public:
    //! @brief The internal build type.
    using build_type = details::promote<parent_build_type>;

    //! @brief Default constructor.
    split() = default;

    //! @brief Row processing.
    template <typename R>
    split& operator<<(R const& row) {
        key_type k = row;
        m_plotters[approx_impl(k, B{})] << row;
        return *this;
    }

    //! @brief Plot building for internal use.
    build_type build() const {
        std::map<key_type, parent_build_type> m;
        for (auto const& x : m_plotters)
            m.emplace(x.first, x.second.build());
        build_type res;
        build_impl(res, m);
        return res;
    }

  private:
    //! @brief No approximations without buckets.
    key_type& approx_impl(key_type& k, std::ratio<0>) {
        return k;
    }
    //! @brief Approximating the key to the closest bucket multiple.
    template <intmax_t num, intmax_t den, typename = std::enable_if_t<num != 0>>
    key_type& approx_impl(key_type& k, std::ratio<num,den>) {
        intmax_t n = std::get<0>(k) * den / num + 0.5;
        std::get<0>(k) = double(n*num)/den;
        return k;
    }
    //! @brief Single plot building.
    void build_impl(std::array<plot, 1>& res, std::map<key_type, parent_build_type>& m) const {
        res[0].xname = common::details::strip_namespaces(common::type_name<S>());
        std::set<std::string> units;
        if (m.size()) for (point const& q : m.begin()->second) {
            if (q.unit.size()) units.insert(q.unit);
            res[0].yvals.emplace_back(q.source, std::vector<double>{});
        }
        for (std::string const& s : units) {
            res[0].yname += (res[0].yname.size() ? "/" : "") + s;
        }
        if (units.empty()) res[0].yname = "y";
        for (auto const& p : m) {
            res[0].xvals.push_back(std::get<0>(p.first));
            for (size_t i = 0; i < p.second.size(); ++i)
                res[0].yvals[i].second.push_back(p.second[i].value);
        }
    }
    //! @brief Multiple plot building.
    void build_impl(std::vector<plot>& res, std::map<key_type, parent_build_type>& m) const {
        for (auto& p : m) {
            std::stringstream ss;
            p.first.print(ss, common::assignment_tuple);
            p.second[0].title = ss.str();
            res.push_back(std::move(p.second[0]));
        }
    }
    //! @brief Single page building.
    void build_impl(std::array<page, 1>& res, std::map<key_type, parent_build_type>& m) const {
        res[0].rows = m.size();
        res[0].cols = m.size() ? m.begin()->second.size() : 0;
        for (auto& p : m) {
            std::stringstream ss;
            p.first.print(ss, common::assignment_tuple);
            for (auto& q : p.second) {
                q.title = ss.str() + (q.title.size() ? ", " : "") + q.title;
                res[0].plots.push_back(std::move(q));
            }
        }
    }
    //! @brief Multiple page building.
    void build_impl(std::vector<page>& res, std::map<key_type, parent_build_type>& m) const {
        for (auto& p : m) {
            std::stringstream ss;
            p.first.print(ss, common::assignment_tuple);
            for (auto& q : p.second) {
                q.title = ss.str() + (q.title.size() ? ", " : "") + q.title;
                res.push_back(std::move(q));
            }
        }
    }

    //! @brief The plotter.
    std::map<key_type, P> m_plotters;
};


}


}

#endif // FCPP_COMMON_PLOT_H_
