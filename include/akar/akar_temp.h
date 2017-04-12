#pragma once

#include <vector>
#include <string>
#include <iterator>
#include <sstream>
#include <thread>
#include <future>
#include <algorithm>
#include <chrono>
#include <string>
#include <random>
#include <time.h>

using namespace std;

template<typename T>
void num_to_str (T& number, string& str)
{
    ostringstream convert;
    convert << number;
    str = convert.str();
    //or..u know... std::to_string
}

template <typename T>
void str_to_num (string& str, T& number)
{
    istringstream convert(str);
    convert >> number;
    //or..u know... stoi
}

template <typename read_iter, typename element>
read_iter find_it(read_iter begin, read_iter end, const element& x)
{
	while (begin != end && *begin != x)
		++begin;

	return begin;
}

template <typename read_iter, typename write_iter>
void copy_it(read_iter begin, read_iter end, write_iter destination)
{
	while (begin != end)
		*destination++ = *begin++;
}

template <typename write_iter, typename element>
void replace_it(write_iter begin, write_iter end, const element& x, const element& y)
{
	while (begin != end) {
		if (*begin == x)
			*begin = y;
		++begin;
	}
}

template <typename random_access_iter, typename element>
bool binary_search_it(random_access_iter begin, random_access_iter end, const element& x)
{
	while (begin < end) {
		random_access_iter mid = begin + (end - begin) / 2;
		if (x < *mid)
			end = mid;
		else if (x > *mid)
			begin = mid + 1;
		else return true;
	}
	return false;
}

template <typename WriteIter, typename UnaryPredicate>
WriteIter move_front(WriteIter begin, WriteIter end,  UnaryPredicate pred)
{
    while(begin != end) {
        if (!pred(*begin)) {
            do  --end;
            while ((!pred(*end)) && end != begin);
            if (end != begin)
                iter_swap(begin, end);
            else
                return begin;
        }
        ++begin;
    }
    return begin;
}

class Stopwatch
{
typedef chrono::high_resolution_clock clock;
public:
    void on() { start = clock::now(); }
    void stop() { finish = clock::now(); }
    double elapsed() { return chrono::duration_cast<chrono::milliseconds>(finish - start).count(); }
private:
    clock::time_point start;
    clock::time_point finish;
};

void sleep(const double& sleep_dur)
{
    Stopwatch stopwatch;
    while(stopwatch.elapsed() < sleep_dur);

    //or..u know... std::this_thread::sleep_for(std::chrono::seconds(1));
}

int rand_eng(const int& x)
{
    static mt19937 mt(1729);
    uniform_int_distribution<int> dist(0, x-1);
    return dist(mt);
}

int rand_eng(const int& x, const int& y)
{
    static mt19937 mt(1729);
    uniform_int_distribution<int> dist(x, y-1);
    return dist(mt);
}

template<typename WriteIter>
void range_fill (WriteIter begin, WriteIter end, const int& boundStart)
{
    for(int i=boundStart; begin != end; ++i)
        *begin++ = i;
}

template<typename WriteIter>
void range_fill (WriteIter begin, WriteIter end, const int& boundStart, const int& boundFinish)
{
    if (boundFinish > boundStart)
        for(int i=boundStart; begin != end &&  i < boundFinish; ++i)
            *begin++ = i;
    else
        for(int i=boundStart; begin != end &&  i > boundFinish; --i)
            *begin++ = i;
}

template<typename WriteIter, typename function>
void range_fill (WriteIter begin, WriteIter end, const int& boundStart, const int& boundFinish, function num_gen)
{
    if (boundFinish > boundStart)
        for(int i=boundStart; begin != end &&  i < boundFinish; ++i)
            *begin++ = num_gen(boundStart, boundFinish);
    else
        for(int i=boundStart; begin != end &&  i > boundFinish; --i)
            *begin++ = num_gen(boundFinish, boundStart);
}

int intParse (string line, char del = ',')
{
    line.erase(remove(line.begin(), line.end(), del), line.end());
    return stoi(line);
}

template<typename WriteIter>
void quicksort(WriteIter begin, WriteIter end)
{
    auto diff = distance(begin, end);

    if (diff > 2) {

        WriteIter last = end-1;
        WriteIter pivot = rand_eng(diff) + begin;
        auto pivotVal = *pivot;

        iter_swap(pivot, last);
        WriteIter it = move_front(begin, last, [&](decltype(pivotVal)& x) { return x <= pivotVal; } );
        iter_swap(it, last);

        quicksort(begin, it++);
        quicksort(it, end);
    }

    else if (diff == 2)
        if (*begin > *(begin+1))
            iter_swap(begin, begin+1);
}

template <typename Func, typename... Args>
auto function_timer(int count, Func fn, Args && ... args)-> typename enable_if<!is_same<decltype(fn(forward<Args>(args)...)), void>::value, pair<decltype(fn(args...)), double>>::type
{
    static_assert(!is_void<decltype(fn(args...))>::value, "Call timer_void if return type is void!\n");

    Stopwatch stopwatch;
    double total_elapsed = 0.0;
    for(int i=0; i < count; ++i) {
        stopwatch.on();
        fn(args...);
        stopwatch.stop();
        total_elapsed += stopwatch.elapsed();
    }
    auto ret = fn(args...);
    return {ret, total_elapsed / count};
}

template <typename Func, typename ... Args>
auto function_timer(Func fn, Args&& ... args) -> typename enable_if<is_same<decltype(fn(forward<Args>(args)...)), void>::value, double>::type
{
    Stopwatch stopwatch;
    fn(args...);
    return stopwatch.elapsed();
}

template<typename Iter, typename Element>
Iter sentinel_find(Iter begin, Iter end, Element elem)
{
    if (begin == end)
        return end;

    Element prev = *(end -1);
    *(end - 1) = elem;

    for(;; ++begin)
        if (*begin == elem)
            break;

    if (begin == (end - 1) && prev != elem )
        ++begin;

    return begin;
}

double percdiff(double x, double y)
{
    double diff = (x - y) / x * 100;
    return (x < y ? -1 * diff : diff);
}
