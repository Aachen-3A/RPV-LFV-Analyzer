#ifndef TriggerClass_hh
#define TriggerClass_hh

#include <string>
#include <vector>

class Trigger {
 public:
    Trigger(std::string input_trigger_name) :
        trigger_name(input_trigger_name)
    {
        std::vector< std::string > trigger_seeds = Get_trigger_seeds(trigger_name);
    }

    void DoAcceptCuts(bool accep_cuts = true) {
        do_accept_cuts = accep_cuts;
    }

    bool GetDimension() const {
        return two_dimensions;
    }

    bool GetAcceptCuts() const {
        return do_accept_cuts;
    }

    std::string GetPart1Name() const {
        return part_1_name;
    }

    std::string GetPart2Name() const {
        return part_2_name;
    }

 private:

    void split(const std::string &s, char delim, std::vector<std::string> *elems) {
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            elems->push_back(item);
        }
    }

    std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        split(s, delim, &elems);
        return elems;
    }

    std::vector< std::string > Get_trigger_seeds(const std::string input_trigger_name) {
        std::vector<std::string> split_trigger_name = split(input_trigger_name, '_');
        for (std::vector< std::string >::const_iterator it = split_trigger_name.begin(); it != split_trigger_name.end(); it++) {
            std::cout << *it << std::endl;
        }
        return split_trigger_name;
    }

    std::string trigger_name;
    std::string part_1_name;
    std::string part_2_name;
    bool two_dimensions;
    bool do_accept_cuts;
    std::vector< pxl::Particle* > * particles_1;
    std::vector< pxl::Particle* > * particles_2;
};

#endif
