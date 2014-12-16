#ifndef CutClass_hh
#define CutClass_hh

class Cuts
{
    public:
        Cuts() :
            cut_name (""),
            cut_passed (false),
            dimensions (1),
            n_bins_x (1),
            x_min (0),
            x_max (1),
            n_bins_y (0),
            y_min (0),
            y_max (0),
            var_1 (0),
            var_2 (0)
            {}
        Cuts(const std::string name) :
            cut_name (name),
            cut_passed (false),
            dimensions (1),
            n_bins_x (1),
            x_min (0),
            x_max (1),
            n_bins_y (0),
            y_min (0),
            y_max (0),
            var_1 (0),
            var_2 (0)
            {}
        Cuts(const std::string name, int i_n_bins_x, double i_x_min, double i_x_max) :
            cut_name (name),
            cut_passed (false),
            dimensions (1),
            n_bins_x (i_n_bins_x),
            x_min (i_x_min),
            x_max (i_x_max),
            n_bins_y (0),
            y_min (0),
            y_max (0),
            var_1 (0),
            var_2 (0)
            {}
        Cuts(const std::string name, int i_n_bins_x, double i_x_min, double i_x_max, int i_n_bins_y, double i_y_min, double i_y_max) :
            cut_name (name),
            cut_passed (false),
            dimensions (2),
            n_bins_x (i_n_bins_x),
            x_min (i_x_min),
            x_max (i_x_max),
            n_bins_y (i_n_bins_y),
            y_min (i_y_min),
            y_max (i_y_max),
            var_1 (0),
            var_2 (0)
            {}
        Cuts(const Cuts& a)
        {}
        ~Cuts()
        {}
        void SetPassed(bool passed)
        {
            cut_passed = passed;
        }
        void SetVars(double i_var_1)
        {
            var_1 = i_var_1;
        }
        void SetVars(double i_var_1, double i_var_2)
        {
            var_1 = i_var_1;
            var_2 = i_var_2;
        }
        int dim()
        {
            return dimensions;
        }
        int bx()
        {
            return n_bins_x;
        }
        int by()
        {
            return n_bins_y;
        }
        double xmi()
        {
            return x_min;
        }
        double xma()
        {
            return x_max;
        }
        double ymi()
        {
            return y_min;
        }
        double yma()
        {
            return y_max;
        }
        double v1()
        {
            return var_1;
        }
        double v2()
        {
            return var_2;
        }
        bool pass()
        {
            return cut_passed;
        }

    private:
        std::string cut_name;
        bool cut_passed;
        int dimensions;
        int n_bins_x;
        double x_min;
        double x_max;
        int n_bins_y;
        double y_min;
        double y_max;
        double var_1;
        double var_2;
};

#endif
