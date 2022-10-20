// Project Identifier: AD48FB4835AF347EB0CA8009E24C3B13F8519882

#include <string>
#include <cmath>
#include <vector>
#include <queue>
#include <iostream>

using namespace std;

class Planet {
    private:
    
    struct Deployment {
        uint32_t timestamp;
        uint32_t general_id;
        uint32_t force;
        mutable uint32_t quantity;
        uint32_t uniqueId;

        Deployment() {
            timestamp = 0;
            general_id = 0;
            force = 0;
            quantity = 0;
            uniqueId = 0;
        }

        Deployment(uint32_t t_in, uint32_t g_in, uint32_t f_in, 
                   uint32_t q_in, uint32_t id_in) : 
                    timestamp(t_in), general_id(g_in), force(f_in), 
                    quantity(q_in), uniqueId(id_in) {}
    };

    struct JediFunctor {
        bool operator()(const Deployment &d1, const Deployment &d2) {
            if (d1.force != d2.force) {
                return d1.force > d2.force;
            }
            return d1.uniqueId > d2.uniqueId;
        }
    };

    struct SithFunctor {
        bool operator()(const Deployment &d1, const Deployment &d2) {
            if (d1.force != d2.force) {
                return d1.force < d2.force;
            }
            return d1.uniqueId > d2.uniqueId;
        }
    };

    enum class State {Initial, SeenFirst, SeenBoth, MaybeBetter};
    struct Movie {
        uint32_t jedi_force;
        uint32_t jedi_time;
        uint32_t sith_force;
        uint32_t sith_time;
        uint32_t maybe_force;
        uint32_t maybe_time;
        State state;

        Movie() {
            jedi_force = 0;
            jedi_time = 0;
            sith_force = 0;
            sith_time = 0;
            maybe_force = 0;
            maybe_time = 0;
            state = State::Initial;
        }

    };

    priority_queue<Deployment, vector<Deployment>, JediFunctor> jediPQ;
    priority_queue<Deployment, vector<Deployment>, SithFunctor> sithPQ;
    //for median
    priority_queue<uint32_t, vector<uint32_t> > lows;
    priority_queue<uint32_t, vector<uint32_t>, std::greater<uint32_t> > highs;
    //for general
    vector<uint32_t> death_counts;
    //for watcher 
    Movie attack;
    Movie ambush;

    uint32_t planet_num;
    uint32_t battles = 0;

    static bool verbose;
    static bool median;
    static bool general_mode;
    static bool watcher;

    public:

    static void set_verbose() {
        verbose = true;
    }

    static void set_median() {
        median = true;
    }

    static bool get_median() {
        return median;
    }

    static void set_general_mode() {
        general_mode = true;
    }

    static bool get_general_mode() {
        return general_mode;
    }

    static void set_watcher() {
        watcher = true;
    }

    static bool get_watcher() {
        return watcher;
    }

    void set_planet_num(uint32_t i) {
        planet_num = i;
    }

    uint32_t get_battles() {
        return battles;
    }

    void resize_death_counts(uint32_t sz) {
        death_counts.resize(sz);
    }

    void make_deployment(uint32_t time, uint32_t gen_num, uint32_t force_num, 
                         uint32_t q, uint32_t id, bool jedi_dep) {
        Deployment d(time, gen_num, force_num, q, id);
        if (jedi_dep) {
            jediPQ.push(d);
        }
        else {
            sithPQ.push(d);
        }
        if (watcher) {
            make_ambush(jedi_dep, d);
            make_attack(jedi_dep, d);
        }
    }

    void balance_check() {
        if (lows.size() > highs.size() + 1) {
            highs.push(lows.top());
            lows.pop();
        }
        else if (highs.size() > lows.size() + 1) {
            lows.push(highs.top());
            highs.pop();
        }
    }

    void fight() { 
        while (!jediPQ.empty() && !sithPQ.empty()) {

            if (jediPQ.top().force > sithPQ.top().force) {
                break;
            }            

            else {
                uint32_t smaller = min(jediPQ.top().quantity, sithPQ.top().quantity);
                uint32_t larger = max(jediPQ.top().quantity, sithPQ.top().quantity);
                ++battles;

                if (verbose) {
                    cout << "General " << sithPQ.top().general_id << "'s battalion attacked General " 
                            << jediPQ.top().general_id << "'s battalion on planet " << planet_num 
                            << ". " << (smaller * 2) << " troops were lost.\n";
                }

                if (general_mode) {
                    death_counts[sithPQ.top().general_id] += smaller;
                    death_counts[jediPQ.top().general_id] += smaller;
                }
                
                if (median) {
                    uint32_t count = smaller * 2;
                    if (lows.empty()) {
                        lows.push(count);
                    }
                    else {
                        if (count < lows.top()) {
                            lows.push(count);
                        }
                        else {
                            highs.push(count);
                        }
                    }
                    balance_check();  
                }

                if (smaller == larger) {
                    jediPQ.pop();
                    sithPQ.pop();
                }
                else if (smaller == jediPQ.top().quantity) {
                    jediPQ.pop();
                    sithPQ.top().quantity -= smaller;
                }
                else if (smaller == sithPQ.top().quantity) {
                    sithPQ.pop();
                    jediPQ.top().quantity -= smaller;
                }
            }
        }
    }

     void make_ambush(bool jedi_dep, Deployment &d) {
        if (ambush.state == State::Initial) {
            if (!jedi_dep) {
                ambush.state = State::SeenFirst;
                ambush.sith_force = d.force;
                ambush.sith_time = d.timestamp;
            }
        }

        else if (ambush.state == State::SeenFirst) {
            if (jedi_dep) {
                if (d.force <= ambush.sith_force) {
                    ambush.state = State::SeenBoth;
                    ambush.jedi_force = d.force;
                    ambush.jedi_time = d.timestamp;
                }
            }
            else if (d.force > ambush.sith_force) {
                ambush.sith_force = d.force;
                ambush.sith_time = d.timestamp;
            }
        }

        else if (ambush.state == State::SeenBoth) {
            if (!jedi_dep) {
                if (d.force > ambush.sith_force) {
                    ambush.state = State::MaybeBetter;
                    ambush.maybe_force = d.force;
                    ambush.maybe_time = d.timestamp;
                }
            }
            else if (d.force < ambush.jedi_force) {
                ambush.jedi_force = d.force;
                ambush.jedi_time = d.timestamp;
            }
        }

        else if (ambush.state == State::MaybeBetter) {
            if (jedi_dep) {
                if (d.force <= ambush.maybe_force) {
                    uint32_t cur_dif = ambush.sith_force - ambush.jedi_force;
                    uint32_t new_dif = ambush.maybe_force - d.force;
                    if (new_dif > cur_dif) {
                        ambush.state = State::SeenBoth;
                        ambush.sith_force = ambush.maybe_force;
                        ambush.sith_time = ambush.maybe_time;
                        ambush.jedi_force = d.force;
                        ambush.jedi_time = d.timestamp;
                    }
                }
            }
            else if (d.force > ambush.maybe_force) {
                ambush.maybe_force = d.force;
                ambush.maybe_time = d.timestamp;
            }
        }
    }

    void make_attack(bool jedi_dep, Deployment &d) {
        if (attack.state == State::Initial) {
            if (jedi_dep) {
                attack.state = State::SeenFirst;
                attack.jedi_force = d.force;
                attack.jedi_time = d.timestamp;
            }
        }

        else if (attack.state == State::SeenFirst) {
            if (!jedi_dep) {
                if (d.force >= attack.jedi_force) {
                    attack.state = State::SeenBoth;
                    attack.sith_force = d.force;
                    attack.sith_time = d.timestamp;
                }
            }
            else if (d.force < attack.jedi_force) {
                attack.jedi_force = d.force;
                attack.jedi_time = d.timestamp;
            }
        }

        else if (attack.state == State::SeenBoth) {
            if (jedi_dep) {
                if (d.force < attack.jedi_force) {
                    attack.state = State::MaybeBetter;
                    attack.maybe_force = d.force;
                    attack.maybe_time = d.timestamp;
                }
            }
            else if (d.force > attack.sith_force) {
                attack.sith_force = d.force;
                attack.sith_time = d.timestamp;
            }
        }

        else if (attack.state == State::MaybeBetter) {
            if (!jedi_dep) {
                if (d.force >= attack.maybe_force) {
                    uint32_t cur_dif = attack.sith_force - attack.jedi_force;
                    uint32_t new_dif = d.force - attack.maybe_force;
                    if (new_dif > cur_dif) {
                        attack.state = State::SeenBoth;
                        attack.sith_force = d.force;
                        attack.sith_time = d.timestamp;
                        attack.jedi_force = attack.maybe_force;
                        attack.jedi_time = attack.maybe_time;
                    }
                }
            }
            else if (d.force < attack.maybe_force) {
                    attack.maybe_force = d.force;
                    attack.maybe_time = d.timestamp;
            }
        }
    }

    uint32_t get_death_counts(uint32_t gen_num) {
        return death_counts[gen_num];
    }

    void print_watcher() {
        int neg = -1;
        int zero = 0;
        if (ambush.state == State::SeenBoth || ambush.state == State::MaybeBetter) {
            cout << "A movie watcher would enjoy an ambush on planet " << planet_num 
                 << " with Sith at time " << ambush.sith_time << " and Jedi at time " 
                 << ambush.jedi_time << " with a force difference of " 
                 << ambush.sith_force - ambush.jedi_force << ".\n";
        }
        else {
            cout << "A movie watcher would enjoy an ambush on planet " << planet_num 
                 << " with Sith at time " << neg << " and Jedi at time " << neg 
                 << " with a force difference of " << zero << ".\n";
        }
        if (attack.state == State::SeenBoth || attack.state == State::MaybeBetter) {
            cout << "A movie watcher would enjoy an attack on planet " << planet_num 
                 << " with Jedi at time " << attack.jedi_time << " and Sith at time " 
                 << attack.sith_time << " with a force difference of " 
                 << attack.sith_force - attack.jedi_force << ".\n";
        }
        else {
            cout << "A movie watcher would enjoy an attack on planet " << planet_num 
                 << " with Jedi at time " << neg << " and Sith at time " << neg 
                 << " with a force difference of " << zero << ".\n";
        }
    }

    void print_median(uint32_t time) {
        if (!lows.empty()) {
            uint32_t med;
            if (lows.size() > highs.size()) {
                med = lows.top();
            }
            else if (highs.size() == lows.size()) {
                med = (highs.top() + lows.top()) / 2;
            }
            else {
                med = highs.top();
            }
            cout << "Median troops lost on planet " << planet_num 
                 << " at time " << time << " is " << med << ".\n";
        }
    }
};