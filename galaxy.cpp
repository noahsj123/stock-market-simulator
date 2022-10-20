// Project Identifier: AD48FB4835AF347EB0CA8009E24C3B13F8519882

//TODO: Move things into planet.cpp and include it
#include "planet.cpp"
#include "P2random.h"
#include <getopt.h>
#include <string>
#include <cmath>
#include <vector>
#include <queue>
#include <iostream>

using namespace std;

bool Planet::verbose = false;
bool Planet::median = false;
bool Planet::general_mode = false;
bool Planet::watcher = false;

class Galaxy {
    private:
        vector<Planet> planets;

        struct General {
            uint32_t jedi_troops;
            uint32_t sith_troops;
            uint32_t killed;

            General() {
                jedi_troops = 0;
                sith_troops = 0;
                killed = 0;
            }
        };

        vector<General> generals;

        string inputMode;

        uint32_t num_battles;
        uint32_t num_planets;
        uint32_t num_generals;

    public:

    Galaxy() {
        string inputMode = "";
        num_battles = 0;
        num_planets = 0;
        num_generals = 0;
    }

    void set_num_battles() {
        for (size_t i = 0; i < planets.size(); ++i) {
            num_battles += planets[i].get_battles();
        } 
    }

    uint32_t get_num_battles() {
        return num_battles;
    } 

    void get_options (int argc, char * argv[]) {
        int gotopt = 0;
        int option_index = 0;

        option long_opts [] = {
            {"verbose",      no_argument, nullptr, 'v'},
            {"median",       no_argument, nullptr, 'm'},
            {"general-eval", no_argument, nullptr, 'g'},
            {"watcher",      no_argument, nullptr, 'w'}
        };

        while ((gotopt = getopt_long(argc, argv, "vmgw", long_opts, &option_index)) != -1) {
            switch (gotopt) {
            case 'v':
                Planet::set_verbose();
                break;
            case 'm':
                Planet::set_median();
                break;
            case 'g':
                Planet::set_general_mode();
                break;
            case 'w':
                Planet::set_watcher();
                break;
            default:
                cerr << "Error: invalid option" << endl;
                exit(1);
            }
        }
    }

    void read_input() {
        string comment;
        getline(cin, comment);

        string s;
        uint32_t num_gen;
        uint32_t num_plan;

        while (cin >> s) {
            if (s == "DL") {
                inputMode = "DL";
            }
            else if (s == "PR") {
                inputMode = "PR";
            }
            else if (s == "NUM_GENERALS:") {
                cin >> num_gen;
                num_generals = num_gen;
                if (planets[0].get_general_mode()) {
                    generals.resize(num_generals);
                }
            }
            else if (s == "NUM_PLANETS:") {
                cin >> num_plan;
                num_planets = num_plan;
                planets.resize(num_planets);
                break;
            }
        }

        stringstream ss;
        if (inputMode == "PR") {
            string junk;
            uint32_t seed;
            uint32_t num_deploys;
            uint32_t rate;
            cin >> junk >> seed;
            cin >> junk >> num_deploys;
            cin >> junk >> rate;
            P2random::PR_init(ss, seed, num_generals, num_planets, num_deploys, rate);
        }

        istream &inputStream = inputMode == "PR" ? ss : cin;

        uint32_t time;
        string side;
        char junk;
        int32_t gen_num;
        int32_t plan_num;
        int32_t force_num;
        int32_t q;
        uint32_t id = 0;

        uint32_t old_time = 0;

        bool gen_mode = false;
        if (planets[0].get_general_mode()) {
            gen_mode = true;
            for (size_t i = 0; i < planets.size(); ++i) {
                planets[i].resize_death_counts(num_generals);
            }
        }
        bool med = false;
        if (planets[0].get_median()) {
            med = true;
        }

        for (uint32_t i = 0; i < planets.size(); ++i) {
            planets[i].set_planet_num(i);
        }

        while (inputStream >> time >> side >> junk >> gen_num >> junk 
                >> plan_num >> junk >> force_num >> junk >> q) {

            bool jedi_dep = false;
            if (side.at(0) == 'J') {
                jedi_dep = true;
            }

            if (static_cast<uint32_t>(gen_num) > num_generals - 1 || gen_num < 0) {
                cerr << "Error: Invalid general id\n";
                exit(1);
            }
            if (static_cast<uint32_t>(plan_num) > num_planets - 1 || plan_num < 0) {
                cerr << "Error: Invalid planet number\n";
                exit(1);
            }
            if (force_num <= 0) {
                cerr << "Error: Invalid force sensitivity\n";
                exit(1);
            }
            if (q <= 0) {
                cerr << "Error: Invalid number of troops\n";
                exit(1);
            }
            if (time < old_time) {
                cerr << "Error: Invalid timestamp\n";
                exit(1);
            }

            if (time != old_time) {
                if (med) {
                    for (size_t i = 0; i < planets.size(); ++i) {
                        planets[i].print_median(old_time);
                    }
                }
                old_time = time;
            }

            planets[static_cast<uint32_t>(plan_num)].make_deployment(time, static_cast<uint32_t>(gen_num), 
                                                                    static_cast<uint32_t>(force_num), 
                                                                    static_cast<uint32_t>(q), id++, jedi_dep);
            if (gen_mode) {
                if (jedi_dep) {
                    generals[static_cast<uint32_t>(gen_num)].jedi_troops += static_cast<uint32_t>(q);
                }
                else {
                    generals[static_cast<uint32_t>(gen_num)].sith_troops += static_cast<uint32_t>(q);
                }
            }
            planets[static_cast<uint32_t>(plan_num)].fight();
        }
        //TODO: make sure I need this if statements
        if (med) {
            for (size_t i = 0; i < planets.size(); ++i) {
                planets[i].print_median(time);
            }
        }
    }    

    void print_gen_mode() {
        if (planets[0].get_general_mode()) {
            cout << "---General Evaluation---\n";
            for (size_t plan = 0; plan < planets.size(); ++plan) {
                for (size_t gen = 0; gen < generals.size(); ++gen) {
                    generals[gen].killed += planets[plan].get_death_counts(static_cast<uint32_t>(gen));
                }
            }
            for (size_t i = 0; i < generals.size(); ++i) {
                uint32_t alive = generals[i].jedi_troops + generals[i].sith_troops;
                uint32_t dead = alive - generals[i].killed;
                cout << "General " << i << " deployed " << generals[i].jedi_troops 
                        << " Jedi troops and " << generals[i].sith_troops << " Sith troops, and " 
                        << dead << '/' << alive << " troops survived.\n";
            }
        }
    }

    void print_watcher_mode() {
        if (planets[0].get_watcher()) {
            cout << "---Movie Watcher---\n";
            for (size_t i = 0; i < planets.size(); ++i) {
                planets[i].print_watcher();
            }
        }
    }

};

int main(int argc, char *argv[]) {
    Galaxy g;
    g.get_options(argc, argv);
    cout << "Deploying troops...\n";
    g.read_input();
    g.set_num_battles();
    cout << "---End of Day---\nBattles: " << g.get_num_battles() << '\n';
    g.print_gen_mode();
    g.print_watcher_mode();
    return 0;
}