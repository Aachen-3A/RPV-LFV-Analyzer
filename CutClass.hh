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
            var_2 (0),
            x_title (""),
            y_title ("")
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
            var_2 (0),
            x_title (""),
            y_title ("")
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
            var_2 (0),
            x_title (""),
            y_title ("")
            {}
        Cuts(const std::string name, int i_n_bins_x, double i_x_min, double i_x_max, std::string i_x_title) :
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
            var_2 (0),
            x_title (i_x_title),
            y_title ("")
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
            var_2 (0),
            x_title (""),
            y_title ("")
            {}
        Cuts(const std::string name, int i_n_bins_x, double i_x_min, double i_x_max, int i_n_bins_y, double i_y_min, double i_y_max, std::string i_x_title, std::string i_y_title) :
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
            var_2 (0),
            x_title (i_x_title),
            y_title (i_y_title)
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
        int dim() const
        {
            return dimensions;
        }
        int bx() const
        {
            return n_bins_x;
        }
        int by() const
        {
            return n_bins_y;
        }
        double xmi() const
        {
            return x_min;
        }
        double xma() const
        {
            return x_max;
        }
        double ymi() const
        {
            return y_min;
        }
        double yma() const
        {
            return y_max;
        }
        double v1() const
        {
            return var_1;
        }
        double v2() const
        {
            return var_2;
        }
        bool pass() const
        {
            return cut_passed;
        }
        std::string xt() const
        {
            return x_title;
        }
        std::string yt() const
        {
            return y_title;
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
        std::string x_title;
        std::string y_title;
};

#endif
