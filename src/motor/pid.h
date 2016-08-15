#ifndef PID_H
#define PID_H

class PID {
    public:
        PID(double Dt, double Kp, double Ki, double Kd);
        void update(double sp, double pv);
        void setLimits(double lower, double upper);         // output will not surpass these limits
        void setDampening(double low, double high);         // integral term will be damped in this range
        void setRollover(double low, double high);          // rollover from one value to another (e.g. 360 to 0 for degrees)
        void setDeadzone(double low, double high);          // the "zero" value at which the system responds
        void setIntegralRange(double low, double high);     // the integral term will not build up outside of this range
        void setTuningMode(bool mode) { tuning = mode; }    // experimental interactive tuning mode (not tested!)
        void changeConstants(double p, double i, double d);
        double getOutput() { return output; }
    private:
        bool outputLimits;
        bool dampening;
        bool rollover;
        bool deadzone;
        bool intRange;
        bool tuning;
        bool initialized;
        double setPointOld;
        double dt, Kp, Ki, Kd;
        double lower, upper;
        double dampLow, dampHigh;
        double rollLow, rollHigh;
        double deadLow, deadHigh;
        double intLow, intHigh;
        double setPoint;
        double proccessValue;
        double error, pError;
        double integral;
        double derivative;
        double output;
};

#endif
