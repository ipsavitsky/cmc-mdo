#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <tuple>
#include <vector>

using weight_t = unsigned int;
using price_t = double;

struct object {
    weight_t weight;
    price_t price;
};

enum class action { undefined, add, not_add };

class knapsack_problem {
  private:
    weight_t C_bound;

    std::vector<object> all_objects;

    struct subsolution {
        action v;
        ptrdiff_t prev_index;
        weight_t weight;
        price_t price;

        bool operator>=(const subsolution &other) const {
            return (weight >= other.weight) && (price <= other.price);
        }
    };

  public:
    knapsack_problem(std::vector<object> objects, weight_t knapsack_bound)
        : all_objects{objects}, C_bound{knapsack_bound} {}

    std::vector<action> solve() {
        using solution_vector = std::vector<subsolution>;
        std::vector<solution_vector> solution_list{
            solution_vector{subsolution{.v = action::undefined,
                                        .prev_index = -1,
                                        .weight = 0,
                                        .price = 0}}};
        solution_list.reserve(all_objects.size() + 1);
        for (const object &cur : all_objects) {
            const solution_vector &last_sols = solution_list.back();
            solution_vector neg_sub;
            solution_vector pos_sub;
            neg_sub.reserve(last_sols.size());
            pos_sub.reserve(last_sols.size());
            for (solution_vector::const_iterator cur_ss = last_sols.cbegin();
                 cur_ss != last_sols.cend(); ++cur_ss) {
                subsolution new_sub = *cur_ss;
                new_sub.v = action::not_add;
                new_sub.prev_index = std::distance(last_sols.begin(), cur_ss);
                neg_sub.push_back(new_sub);

                new_sub.v = action::add;
                new_sub.price += cur.price;
                new_sub.weight += cur.weight;

                if (new_sub.weight <= C_bound) {
                    pos_sub.push_back(new_sub);
                }
            }

            solution_vector new_sol;
            new_sol.reserve(neg_sub.size() + pos_sub.size());
            solution_vector::const_iterator neg_iter = neg_sub.cbegin();
            solution_vector::const_iterator pos_iter = pos_sub.cbegin();

            while ((neg_iter != neg_sub.cend()) &&
                   (pos_iter != pos_sub.cend())) {
                if (*neg_iter >= *pos_iter) {
                    ++neg_iter;
                } else if (*pos_iter >= *neg_iter) {
                    ++pos_iter;
                } else {
                    if (neg_iter->weight < pos_iter->weight) {
                        new_sol.push_back(*neg_iter);
                        ++neg_iter;
                    } else {
                        new_sol.push_back(*pos_iter);
                        ++pos_iter;
                    }
                }
            }

            std::copy(neg_iter, neg_sub.cend(), std::back_inserter(new_sol));
            std::copy(pos_iter, pos_sub.cend(), std::back_inserter(new_sol));

            solution_list.push_back(new_sol);
        }
        std::vector<action> res;
        res.reserve(all_objects.size());
        solution_vector::const_iterator unravel =
            std::prev(solution_list.back().cend());
        for (std::vector<solution_vector>::const_reverse_iterator i =
                 solution_list.crbegin();
             i != std::prev(solution_list.crend()); ++i) {
            res.push_back(unravel->v);
            unravel = std::next(std::next(i)->cbegin(), unravel->prev_index);
        }
        std::reverse(res.begin(), res.end());
        return res;
    }
};

struct multidim_object {
    std::vector<weight_t> weights;
    price_t price;
};

class multidim_knapsack {
  private:
    std::vector<weight_t> C_bounds;

    std::vector<multidim_object> all_objects;

    double absolute(const std::vector<double> vec) {
        return std::sqrt(
            std::inner_product(vec.begin(), vec.end(), vec.begin(), 0.));
    }

  public:
    multidim_knapsack(std::vector<multidim_object> objects,
                      std::vector<weight_t> weight_bounds)
        : all_objects{objects}, C_bounds{weight_bounds} {}

    std::tuple<double, std::vector<double>> lagrange_approximation() {
        std::vector<double> lambda_vector(C_bounds.size() - 1, 1);
        std::vector<object> new_objects;
        std::vector<action> new_solution;
        for (size_t i = 0; i < 100000; ++i) {
            new_objects.clear();
            new_objects.reserve(all_objects.size());
            for (int j = 0; j < all_objects.size(); ++j) {
                double coef = 0;
                for (int k = 0; k < lambda_vector.size(); ++k) {
                    coef += lambda_vector[k] * all_objects[j].weights[k];
                }
                new_objects.push_back(
                    object{.weight = all_objects[j].weights.back(),
                           .price = std::max(all_objects[j].price - coef, 0.)});
            }
            knapsack_problem new_problem{new_objects, C_bounds.back()};
            new_solution = new_problem.solve();

            std::vector<double> D;

            for (int j = 0; j < lambda_vector.size(); ++j) {
                D.push_back(C_bounds[j]);
                for (int k = 0; k < all_objects.size(); ++k) {
                    if (new_solution[k] == action::add) {
                        D.back() -= all_objects[k].weights[j];
                    }
                }
            }

            for (int j = 0; j < lambda_vector.size(); ++j) {
                if (absolute(D) > 1e-4) {
                    lambda_vector[j] -= (D[j] / absolute(D)) / (i + 1);
                }
                lambda_vector[j] = std::max(lambda_vector[j], 0.);
            }
        }
        double result = 0;
        for (int i = 0; i < new_objects.size(); ++i) {
            if (new_solution[i] == action::add) {
                result += new_objects[i].price;
            }
        }
        for (int i = 0; i < lambda_vector.size(); ++i) {
            result += lambda_vector[i] * C_bounds[i];
        }
        return std::make_tuple(result, lambda_vector);
    }
};

int main() {
    int n, m;
    std::cin >> n >> m;
    std::vector<multidim_object> test(n);
    std::vector<weight_t> constraints(m);
    for (int i = 0; i < m; ++i) {
        std::cin >> constraints[i];
    }
    for (int i = 0; i < n; ++i) {
        std::cin >> test[i].price;
    }
    for (int i = 0; i < n; ++i) {
        test[i].weights.resize(m);
        for (int j = 0; j < m; ++j) {
            std::cin >> test[i].weights[j];
        }
    }
    multidim_knapsack test_problem{test, constraints};
    auto [l, lambdas] = test_problem.lagrange_approximation();
    std::cout << l << std::endl;
    for (auto lambda : lambdas) {
        std::cout << lambda << std::endl;
    }
}