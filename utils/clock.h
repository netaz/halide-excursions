// A current_time function for use in the tests.  Returns time in
// milliseconds.
#ifndef __CLOCK_H
#define __CLOCK_H

#include <vector>
#include <algorithm>
#include <string>

#define __DEBUG printf

#ifdef _WIN32
extern "C" bool QueryPerformanceCounter(uint64_t *);
extern "C" bool QueryPerformanceFrequency(uint64_t *);
double current_time() {
    uint64_t t, freq;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&freq);
    return (t * 1000.0) / freq;
}
#else
#include <sys/time.h>
double current_time() {
    static bool first_call = true;
    static timeval reference_time;
    if (first_call) {
        first_call = false;
        gettimeofday(&reference_time, NULL);
        return 0.0;
    } else {
        timeval t;
        gettimeofday(&t, NULL);
        return ((t.tv_sec - reference_time.tv_sec)*1000.0 +
                (t.tv_usec - reference_time.tv_usec)/1000.0);
    }
}
#endif


class timings {
public:
    timings(const std::string desc = "") : desc(desc) {}

    void add(double duration) {
        samples.push_back(duration);
    }

    double mean(double &std_dev) {
        max = std::numeric_limits<float>::min();
        min = std::numeric_limits<float>::max();

        auto imax = std::max_element(samples.begin(), samples.end());
        auto imin = std::min_element(samples.begin(), samples.end());
        __DEBUG("here1-1\n");
        samples.erase(imin);
        samples.erase(imax);
        __DEBUG("here1-2\n");
        imax = std::max_element(samples.begin(), samples.end());
        imin = std::min_element(samples.begin(), samples.end());
        __DEBUG("here1-2-1\n");
        samples.erase(imin);
        samples.erase(imax);
        __DEBUG("here1-3\n");
        min = *imin;
        max = *imin;

        double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
        double mean = sum / samples.size();
        __DEBUG("here1-4\n");

        std::vector<double> diff(samples.size());
        std::transform(samples.begin(), samples.end(), diff.begin(), std::bind2nd(std::minus<double>(), mean));
        double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
        std_dev = std::sqrt(sq_sum / samples.size());
        return mean;
    }

    void dump() {
        FILE *fd = fopen("dump.txt", "w");
        assert(fd);
        fprintf(fd, "%s - %lu\n", desc.c_str(), samples.size());
        for (size_t i=0; i<samples.size(); i++) 
            fprintf(fd, "%f\n", samples[i]);
        __DEBUG("here1\n");
        double dev = 0;
        double m = mean(dev);
        __DEBUG("here2\n");
        fprintf(fd, "mean=%f std_dev=%f\n", m, dev);
        fclose(fd);
    }

private:
    std::vector<double> samples; 
    double min, max;
    std::string desc;
};

struct interval {
  timings &t;
  double start;
  interval(timings &t) : t(t), start(current_time()) {}
  ~interval() { t.add(current_time() - start); }
};

#endif // __CLOCK_H