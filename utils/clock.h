// A current_time function for use in the tests.  Returns time in
// milliseconds.
#ifndef __CLOCK_H
#define __CLOCK_H

#include <vector>
#include <algorithm>
#include <string>

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


struct timings {
    std::vector<double> samples; 
    double min, max;
    timings(){}

    void add(double duration) {
        samples.push_back(duration);
    }

    double mean(double &std_dev) {
        max = std::numeric_limits<float>::min();
        min = std::numeric_limits<float>::max();

        auto imax = std::max_element(samples.begin(), samples.end());
        auto imin = std::min_element(samples.begin(), samples.end());
        printf("here1-1\n");
        samples.erase(imin);
        samples.erase(imax);
        printf("here1-2\n");
        imax = std::max_element(samples.begin(), samples.end());
        imin = std::min_element(samples.begin(), samples.end());
        printf("here1-2-1\n");
        samples.erase(imin);
        samples.erase(imax);
        printf("here1-3\n");
        min = *imin;
        max = *imin;

        double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
        double mean = sum / samples.size();
        printf("here1-4\n");

        std::vector<double> diff(samples.size());
        std::transform(samples.begin(), samples.end(), diff.begin(), std::bind2nd(std::minus<double>(), mean));
        double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
        std_dev = std::sqrt(sq_sum / samples.size());
        return mean;
    }

    void dump() {
        printf("Dumping %lu results:\n", samples.size());
        FILE *fd = fopen("dump.txt", "w");
        assert(fd);
        for (size_t i=0; i<samples.size(); i++) 
            fprintf(fd, "%f\n", samples[i]);
        printf("here1\n");
        double dev = 0;
        double m = mean(dev);
        printf("here2\n");
        fprintf(fd, "mean=%f std_dev=%f\n", m, dev);
        fclose(fd);
    }
};

struct interval {
  timings &t;
  double start;
  interval(timings &t) : t(t), start(current_time()) {}
  ~interval() { t.add(current_time() - start); }
};

#endif // __CLOCK_H