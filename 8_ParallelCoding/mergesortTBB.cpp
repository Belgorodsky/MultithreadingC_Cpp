#include <iostream>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <memory>
#include <tbb/tbb.h>

void parse_space_separated_tokens(const std::string &src_filename, tbb::concurrent_vector<int64_t>& result)
{
        std::ifstream f(src_filename, std::ios::binary);
        tbb::concurrent_vector<int64_t> tokens;

        std::copy(
                    std::istream_iterator<int64_t>(f),
                    std::istream_iterator<int64_t>(),
                    std::back_inserter(tokens)
                );

        result = std::move(tokens);
}

void output_sorted(const tbb::concurrent_vector<int64_t>& result, int argc, char *argv[])
{
    constexpr int params_required = 2;
    if (params_required == argc)
    {
        std::copy(
                    result.begin(),
                    result.end(),
                    std::ostream_iterator<int64_t>(std::cout, " ")
        );

        std::cout << std::endl;
    }
    else
    {   
        std::ofstream f(argv[2]);
        std::copy(
                    result.begin(),
                    result.end(),
                    std::ostream_iterator<int64_t>(f, " ")
        );

        f << std::endl;
    }
}

template <typename Iterator>
void mergesort(Iterator from, Iterator to)
{
    static_assert(!std::is_same<typename std::iterator_traits<Iterator>::value_type, void>::value);

    auto n = std::distance(from, to);

    if (1 < n)
    {
        {
            Iterator l_from = from;
            Iterator l_to = l_from;
            std::advance(l_to, n/2);

            Iterator r_from = from;
            std::advance(r_from, n/2);
            Iterator r_to = r_from;
            std::advance(r_to, n-(n/2));

            tbb::parallel_invoke(
                [&]
                {
                    mergesort(l_from, l_to);
                },
                [&]
                {
                    mergesort(r_from, r_to);
                }
            );
        }

        auto tmp_array = std::make_unique<typename Iterator::value_type[]>(n);
        Iterator l_iter = from;
        Iterator l_end = l_iter;
        std::advance(l_end, n/2);
        Iterator r_iter = l_end;
        Iterator& r_end = to;

        auto tmp_iter = tmp_array.get();

        while (l_iter != l_end || r_iter != r_end)
        {
            if (*l_iter < *r_iter)
            {
                *tmp_iter = std::move(*l_iter);
                ++l_iter;
                ++tmp_iter;
            }
            else
            {
                *tmp_iter = std::move(*r_iter);
                ++r_iter;
                ++tmp_iter;
            }

            if (l_iter == l_end)
            {
                std::copy(
                            std::make_move_iterator(r_iter),
                            std::make_move_iterator(r_end),
                            tmp_iter
                );

                break;
            }

            if (r_iter == r_end)
            {
                std::copy(
                            std::make_move_iterator(l_iter),
                            std::make_move_iterator(l_end),
                            tmp_iter
                );

                break;
            }
        }

        std::copy(
                    std::make_move_iterator(tmp_array.get()),
                    std::make_move_iterator(&tmp_array[n]),
                    from
        );
    }
}


int main(int argc, char *argv[])
{
    constexpr int params_required = 2;
    tbb::concurrent_vector<int64_t> tokens;

    if (params_required <= argc)
    {
        parse_space_separated_tokens(argv[1], tokens);
    }
    else
    {
        std::cerr << "requred " << params_required << "params : 1st for in file, 2end for out file" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    mergesort(tokens.begin(), tokens.end());

    output_sorted(tokens, argc, argv);

    return 0;
}
