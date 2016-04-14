#ifndef PID_H
#define PID_H

class PID {
    public:
        PID(double Dt, double Kp, double Ki, double Kd);
        void update(double sp, double pv);
        void setLimits(double lower, double upper);
        void setDampening(double low, double high);
        double getOutput() { return output; }
    private:
        bool outputLimits;
        bool dampening;
        double dt, Kp, Ki, Kd;
        double lower, upper;
        double dampLow, dampHigh;
        double setPoint;
        double proccessValue;
        double error, pError;
        double integral;
        double derivative;
        double output;
};

#endif
