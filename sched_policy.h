#ifndef __SCHED_POLICY_H
#define __SCHED_POLICY_H

class Scheduler {
public:
    virtual void schedule(Halide::Func f, Halide::Var x, Halide::Var y) const = 0;
    virtual void schedule(Halide::Func f1, Halide::Func f2, Halide::Var x, Halide::Var y) const = 0;
    //virtual void schedule(Halide::Func f1, Halide::Func f2) const = 0;
};

class NoPSched : public Scheduler {
    virtual void schedule(Halide::Func f, Halide::Var x, Halide::Var y) const {}
    virtual void schedule(Halide::Func f1, Halide::Func f2, Halide::Var x, Halide::Var y) const {}
    //virtual void schedule(Halide::Func f1, Halide::Func f2) const {}
};

#endif // __SCHED_POLICY_H