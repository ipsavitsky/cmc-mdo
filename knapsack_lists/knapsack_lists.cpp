#include <algorithm>
#include <iostream>
#include <vector>

using weight_t = unsigned int;
using price_t = unsigned int;

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

    friend std::ostream &operator<<(std::ostream &os,
                                    const knapsack_problem::subsolution &sol);

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
        for (const object &cur : all_objects) {
            const solution_vector &last_sols = solution_list.back();
            solution_vector neg_sub;
            solution_vector pos_sub;
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
        solution_vector::const_iterator unravel = std::prev(solution_list.back().cend());
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

std::ostream &operator<<(std::ostream &os,
                         const knapsack_problem::subsolution &sol) {
    os << '{' << "l:" << sol.prev_index << " w:" << sol.weight
       << " pr:" << sol.price << '}';
    return os;
}

int main() {
    std::size_t n;
    weight_t C;
    std::cin >> n >> C;
    std::vector<object> objects;
    for (int i = 0; i < n; ++i) {
        object new_obj;
        std::cin >> new_obj.weight >> new_obj.price;
        objects.push_back(new_obj);
    }
    knapsack_problem prob(objects, C);
    std::vector<action> res = prob.solve();
    unsigned int sum = 0;
    unsigned int num_of_obj = 0;
    for (int i = 0; i < res.size(); ++i) {
        if(res[i] == action::add) {
            sum += objects[i].price;
            ++num_of_obj;
        }
    }
    std::cout << sum << " " << num_of_obj << std::endl;
    for (int i = 0; i < res.size(); ++i) {
        if(res[i] == action::add) {
            std::cout << i << std::endl;
        }
    }
}