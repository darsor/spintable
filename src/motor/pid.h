#ifndef PID_H
#define PID_H

class PID {
    public:
        PID(double Dt, double Kp, double Ki, double Kd);
        void update(double sp, double pv);
        void setLimits(double lower, double upper);
        void setDampening(double low, double high);
        void setRollover(double low, double high);
        void setDeadzone(double low, double high);
        void setTuningMode(bool mode) { tuning = mode; }
        void changeConstants(double p, double i, double d);
        double getOutput() { return output; }
    private:
        bool outputLimits;
        bool dampening;
        bool rollover;
        bool deadzone;
        bool tuning;
        double setPointOld;
        double dt, Kp, Ki, Kd;
        double lower, upper;
        double dampLow, dampHigh;
        double rollLow, rollHigh;
        double deadLow, deadHigh;
        double setPoint;
        double proccessValue;
        double error, pError;
        double integral;
        double derivative;
        double output;
};

#endif
