#pragma once

#include <vector>
#include <mutex>
#include <atomic>


#include "common.hpp"
#include "glm.hpp"

using namespace glm;

namespace glimac {
    
    enum metric {
        e,
        e0,
        e1,
        e2,
        e3,
        e01,
        e02,
        e03,
        e12,
        e31,
        e23,
        e021,
        e013,
        e032,
        e123,
        e0123
    };

    struct unit
    {
        int e;
    };

    struct vector
    {
        int e0;
        int e1;
        int e2;
        int e3;
    };

    struct bivector
    {
        int e01;
        int e02;
        int e03;
        int e12;
        int e31;
        int e23;
    };

    struct trivector
    {
        int e021;
        int e013;
        int e032;
        int e123;
    };

    struct identity
    {
        int e0123;
    };
    

    struct Space
    {
        unit u;
        vector v;
        bivector b;
        trivector t;
        identity i;
        void Euclidian() {
            // unit
            u.e = 1;

            // vector
            v.e0 =  0;
            v.e1 = +1;
            v.e2 = +1;
            v.e3 = +1;

            // bivector
            b.e01 =  0;
            b.e02 =  0;
            b.e03 =  0;
            b.e12 = -1;
            b.e31 = -1;
            b.e23 = -1;

            // trivector
            t.e021 =  0;
            t.e013 =  0;
            t.e032 =  0;
            t.e123 = -1;
            
            // identity
            i.e0123 = 0;
        }
        /* data */
        int multiplcationTable(metric m1, metric m2) {
            switch (m1)
            {
            case metric::e:
                switch(m2) {
                    case metric::e:
                        return u.e;
                    case metric::e0:
                        return v.e0;
                    case metric::e1:
                        return v.e1;
                    case metric::e2:
                        return v.e2;
                    case metric::e3:
                        return v.e3;
                    case metric::e01:
                        return b.e01;
                    case metric::e02:
                        return b.e02;
                    case metric::e03:
                        return b.e03;
                    case metric::e12:
                        return b.e12;
                    case metric::e31:
                        return b.e31;
                    case metric::e23:
                        return b.e23;
                    case metric::e021:
                        return t.e021;
                    case metric::e013:
                        return t.e013;
                    case metric::e032:
                        return t.e032;
                    case metric::e123:
                        return t.e123;
                    case metric::e0123:
                        return i.e0123;
                }
            case metric::e0:
                switch(m2) {
                    case metric::e:
                        return v.e0;
                    case metric::e0:
                        return 0;
                    case metric::e1:
                        return -b.e01;
                    case metric::e2:
                        return -b.e02;
                    case metric::e3:
                        return -b.e03;
                    case metric::e01:
                        return 0;
                    case metric::e02:
                        return 0;
                    case metric::e03:
                        return 0;
                    case metric::e12:
                        return -t.e021;
                    case metric::e31:
                        return -t.e013;
                    case metric::e23:
                        return -t.e032;
                    case metric::e021:
                        return 0;
                    case metric::e013:
                        return 0;
                    case metric::e032:
                        return 0;
                    case metric::e123:
                        return -i.e0123;
                    case metric::e0123:
                        return 0;
                }
            case metric::e1:
                switch(m2) {
                    case metric::e:
                        return v.e1;
                    case metric::e0:
                        return b.e01;
                    case metric::e1:
                        return u.e;
                    case metric::e2:
                        return -b.e12;
                    case metric::e3:
                        return b.e31;
                    case metric::e01:
                        return v.e0;
                    case metric::e02:
                        return t.e021;
                    case metric::e03:
                        return -t.e013;
                    case metric::e12:
                        return -v.e2;
                    case metric::e31:
                        return v.e3;
                    case metric::e23:
                        return t.e123;
                    case metric::e021:
                        return b.e02;
                    case metric::e013:
                        return b.e03;
                    case metric::e032:
                        return -i.e0123;
                    case metric::e123:
                        return b.e23;
                    case metric::e0123:
                        return -t.e032;
                }
            case metric::e2:
                switch(m2) {
                    case metric::e:
                        return v.e2;
                    case metric::e0:
                        return b.e02;
                    case metric::e1:
                        return b.e01;
                    case metric::e2:
                        return u.e;
                    case metric::e3:
                        return -b.e23;
                    case metric::e01:
                        return -t.e021;
                    case metric::e02:
                        return v.e0;
                    case metric::e03:
                        return t.e032;
                    case metric::e12:
                        return v.e1;
                    case metric::e31:
                        return t.e123;
                    case metric::e23:
                        return -v.e3;
                    case metric::e021:
                        return -b.e01;
                    case metric::e013:
                        return -i.e0123;
                    case metric::e032:
                        return b.e03;
                    case metric::e123:
                        return b.e31;
                    case metric::e0123:
                        return -t.e013;
                }
            case metric::e3:
                switch(m2) {
                    case metric::e:
                        return v.e3;
                    case metric::e0:
                        return b.e03;
                    case metric::e1:
                        return -b.e31;
                    case metric::e2:
                        return b.e23;
                    case metric::e3:
                        return u.e;
                    case metric::e01:
                        return t.e013;
                    case metric::e02:
                        return -t.e032;
                    case metric::e03:
                        return v.e0;
                    case metric::e12:
                        return t.e123;
                    case metric::e31:
                        return -v.e1;
                    case metric::e23:
                        return v.e2;
                    case metric::e021:
                        return -i.e0123;
                    case metric::e013:
                        return b.e01;
                    case metric::e032:
                        return -b.e02;
                    case metric::e123:
                        return b.e12;
                    case metric::e0123:
                        return -t.e021;
                }
            case metric::e01:
                switch(m2) {
                    case metric::e:
                        return b.e01;
                    case metric::e0:
                        return 0;
                    case metric::e1:
                        return -v.e0;
                    case metric::e2:
                        return -t.e021;
                    case metric::e3:
                        return t.e013;
                    case metric::e01:
                        return 0;
                    case metric::e02:
                        return 0;
                    case metric::e03:
                        return 0;
                    case metric::e12:
                        return -b.e02;
                    case metric::e31:
                        return b.e03;
                    case metric::e23:
                        return i.e0123;
                    case metric::e021:
                        return 0;
                    case metric::e013:
                        return 0;
                    case metric::e032:
                        return 0;
                    case metric::e123:
                        return t.e032;
                    case metric::e0123:
                        return 0;
                }
            case metric::e02:
                switch(m2) {
                    case metric::e:
                        return b.e02;
                    case metric::e0:
                        return 0;
                    case metric::e1:
                        return t.e021;
                    case metric::e2:
                        return -v.e0;
                    case metric::e3:
                        return -t.e032;
                    case metric::e01:
                        return 0;
                    case metric::e02:
                        return 0;
                    case metric::e03:
                        return 0;
                    case metric::e12:
                        return b.e01;
                    case metric::e31:
                        return i.e0123;
                    case metric::e23:
                        return -b.e03;
                    case metric::e021:
                        return 0;
                    case metric::e013:
                        return 0;
                    case metric::e032:
                        return 0;
                    case metric::e123:
                        return t.e013;
                    case metric::e0123:
                        return 0;
                }
            case metric::e03:
                switch(m2) {
                    case metric::e:
                        return b.e03;
                    case metric::e0:
                        return 0;
                    case metric::e1:
                        return -t.e013;
                    case metric::e2:
                        return t.e032;
                    case metric::e3:
                        return -v.e0;
                    case metric::e01:
                        return 0;
                    case metric::e02:
                        return 0;
                    case metric::e03:
                        return 0;
                    case metric::e12:
                        return i.e0123;
                    case metric::e31:
                        return -b.e01;
                    case metric::e23:
                        return b.e02;
                    case metric::e021:
                        return 0;
                    case metric::e013:
                        return 0;
                    case metric::e032:
                        return 0;
                    case metric::e123:
                        return t.e021;
                    case metric::e0123:
                        return 0;
                }
            case metric::e12:
                switch(m2) {
                    case metric::e:
                        return b.e12;
                    case metric::e0:
                        return -t.e021;
                    case metric::e1:
                        return v.e2;
                    case metric::e2:
                        return -v.e1;
                    case metric::e3:
                        return t.e123;
                    case metric::e01:
                        return b.e02;
                    case metric::e02:
                        return -b.e01;
                    case metric::e03:
                        return i.e0123;
                    case metric::e12:
                        return -u.e;
                    case metric::e31:
                        return -b.e23;
                    case metric::e23:
                        return b.e31;
                    case metric::e021:
                        return v.e0;
                    case metric::e013:
                        return -t.e032;
                    case metric::e032:
                        return t.e013;
                    case metric::e123:
                        return -v.e3;
                    case metric::e0123:
                        return b.e03;
                }
            case metric::e31:
                switch(m2) {
                    case metric::e:
                        return b.e31;
                    case metric::e0:
                        return -t.e013;
                    case metric::e1:
                        return -v.e3;
                    case metric::e2:
                        return t.e123;
                    case metric::e3:
                        return v.e1;
                    case metric::e01:
                        return -b.e03;
                    case metric::e02:
                        return i.e0123;
                    case metric::e03:
                        return b.e01;
                    case metric::e12:
                        return b.e23;
                    case metric::e31:
                        return -u.e;
                    case metric::e23:
                        return -b.e12;
                    case metric::e021:
                        return t.e032;
                    case metric::e013:
                        return v.e0;
                    case metric::e032:
                        return -t.e021;
                    case metric::e123:
                        return -v.e2;
                    case metric::e0123:
                        return -b.e02;
                }
            case metric::e23:
                switch(m2) {
                    case metric::e:
                        return b.e23;
                    case metric::e0:
                        return -t.e032;
                    case metric::e1:
                        return t.e123;
                    case metric::e2:
                        return v.e3;
                    case metric::e3:
                        return -v.e2;
                    case metric::e01:
                        return i.e0123;
                    case metric::e02:
                        return b.e03;
                    case metric::e03:
                        return -b.e02;
                    case metric::e12:
                        return -b.e31;
                    case metric::e31:
                        return b.e12;
                    case metric::e23:
                        return -u.e;
                    case metric::e021:
                        return -t.e013;
                    case metric::e013:
                        return t.e021;
                    case metric::e032:
                        return v.e0;
                    case metric::e123:
                        return -v.e1;
                    case metric::e0123:
                        return -b.e01;
                }
            case metric::e021:
                switch(m2) {
                    case metric::e:
                        return t.e021;
                    case metric::e0:
                        return 0;
                    case metric::e1:
                        return b.e02;
                    case metric::e2:
                        return -b.e01;
                    case metric::e3:
                        return i.e0123;
                    case metric::e01:
                        return 0;
                    case metric::e02:
                        return 0;
                    case metric::e03:
                        return 0;
                    case metric::e12:
                        return v.e0;
                    case metric::e31:
                        return -t.e032;
                    case metric::e23:
                        return t.e013;
                    case metric::e021:
                        return 0;
                    case metric::e013:
                        return 0;
                    case metric::e032:
                        return 0;
                    case metric::e123:
                        return -b.e03;
                    case metric::e0123:
                        return 0;
                }
            case metric::e013:
                switch(m2) {
                    case metric::e:
                        return t.e013;
                    case metric::e0:
                        return 0;
                    case metric::e1:
                        return -b.e03;
                    case metric::e2:
                        return i.e0123;
                    case metric::e3:
                        return b.e01;
                    case metric::e01:
                        return 0;
                    case metric::e02:
                        return 0;
                    case metric::e03:
                        return 0;
                    case metric::e12:
                        return t.e032;
                    case metric::e31:
                        return v.e0;
                    case metric::e23:
                        return -t.e021;
                    case metric::e021:
                        return 0;
                    case metric::e013:
                        return 0;
                    case metric::e032:
                        return 0;
                    case metric::e123:
                        return -b.e02;
                    case metric::e0123:
                        return 0;
                }
            case metric::e032:
                switch(m2) {
                    case metric::e:
                        return t.e032;
                    case metric::e0:
                        return 0;
                    case metric::e1:
                        return i.e0123;
                    case metric::e2:
                        return b.e03;
                    case metric::e3:
                        return -b.e02;
                    case metric::e01:
                        return 0;
                    case metric::e02:
                        return 0;
                    case metric::e03:
                        return 0;
                    case metric::e12:
                        return -t.e013;
                    case metric::e31:
                        return t.e021;
                    case metric::e23:
                        return v.e0;
                    case metric::e021:
                        return 0;
                    case metric::e013:
                        return 0;
                    case metric::e032:
                        return 0;
                    case metric::e123:
                        return -b.e01;
                    case metric::e0123:
                        return 0;
                }
            case metric::e123:
                switch(m2) {
                    case metric::e:
                        return t.e123;
                    case metric::e0:
                        return i.e0123;
                    case metric::e1:
                        return b.e23;
                    case metric::e2:
                        return b.e31;
                    case metric::e3:
                        return b.e12;
                    case metric::e01:
                        return -t.e032;
                    case metric::e02:
                        return -t.e013;
                    case metric::e03:
                        return -t.e021;
                    case metric::e12:
                        return -v.e3;
                    case metric::e31:
                        return -v.e2;
                    case metric::e23:
                        return -v.e1;
                    case metric::e021:
                        return b.e03;
                    case metric::e013:
                        return b.e02;
                    case metric::e032:
                        return b.e01;
                    case metric::e123:
                        return -u.e;
                    case metric::e0123:
                        return -v.e0;
                }
            case metric::e0123:
                switch(m2) {
                    case metric::e:
                        return i.e0123;
                    case metric::e0:
                        return 0;
                    case metric::e1:
                        return t.e032;
                    case metric::e2:
                        return t.e013;
                    case metric::e3:
                        return t.e021;
                    case metric::e01:
                        return 0;
                    case metric::e02:
                        return 0;
                    case metric::e03:
                        return 0;
                    case metric::e12:
                        return -b.e03;
                    case metric::e31:
                        return -b.e02;
                    case metric::e23:
                        return -b.e01;
                    case metric::e021:
                        return 0;
                    case metric::e013:
                        return 0;
                    case metric::e032:
                        return 0;
                    case metric::e123:
                        return v.e0;
                    case metric::e0123:
                        return 0;
                }
            default:
                break;
            }
        }
    };

    

    // class Space
    // {
    // private:
    //     /* data */
    // public:
    //     Space(/* args */);
    //     ~Space();
    // };
       
}