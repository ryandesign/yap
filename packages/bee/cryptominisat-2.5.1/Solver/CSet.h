/**************************************************************************************************
From: Solver.C -- (C) Niklas Een, Niklas Sorensson, 2004
**************************************************************************************************/

#ifndef CSET_H
#define CSET_H

#include "Vec.h"
#include <limits>
#ifdef _MSC_VER
#include <msvc/stdint.h>
#else
#include <stdint.h>
#endif //_MSC_VER

class Clause;

template <class T>
uint32_t calcAbstraction(const T& ps) {
    uint32_t abstraction = 0;
    for (uint32_t i = 0; i != ps.size(); i++)
        abstraction |= 1 << (ps[i].toInt() & 31);
    return abstraction;
}

//#pragma pack(push)
//#pragma pack(1)
class ClauseSimp
{
    public:
        ClauseSimp(Clause* c, const uint32_t _index) :
        clause(c)
        , index(_index)
        {}
        
        Clause* clause;
        uint32_t index;
};
//#pragma pack(pop)

class CSet {
    vec<uint32_t>       where;  // Map clause ID to position in 'which'.
    vec<ClauseSimp> which;  // List of clauses (for fast iteration). May contain 'Clause_NULL'.
    vec<uint32_t>       free;   // List of positions holding 'Clause_NULL'.
    
    public:
        //ClauseSimp& operator [] (uint32_t index) { return which[index]; }
        void reserve(uint32_t size) { where.reserve(size);}
        uint32_t size(void) const { return which.size(); }
        uint32_t nElems(void) const { return which.size() - free.size(); }
        
        bool add(const ClauseSimp& c) {
            assert(c.clause != NULL);
            where.growTo(c.index+1, std::numeric_limits<uint32_t>::max());
            if (where[c.index] != std::numeric_limits<uint32_t>::max()) {
                return true;
            }
            if (free.size() > 0){
                where[c.index] = free.last();
                which[free.last()] = c;
                free.pop();
            }else{
                where[c.index] = which.size();
                which.push(c);
            }
            return false;
        }
        
        bool exclude(const ClauseSimp& c) {
            assert(c.clause != NULL);
            if (c.index >= where.size() || where[c.index] == std::numeric_limits<uint32_t>::max()) {
                //not inside
                return false;
            }
            free.push(where[c.index]);
            which[where[c.index]].clause = NULL;
            where[c.index] = std::numeric_limits<uint32_t>::max();
            return true;
        }
        
        void clear(void) {
            for (uint32_t i = 0; i < which.size(); i++)  {
                if (which[i].clause != NULL) {
                    where[which[i].index] = std::numeric_limits<uint32_t>::max();
                }
            }
            which.clear();
            free.clear();
        }
        
        class iterator
        {
            public:
                iterator(ClauseSimp* _it) :
                it(_it)
                {}
                
                void operator++()
                {
                    it++;
                }
                
                const bool operator!=(const iterator& iter) const
                {
                    return (it != iter.it);;
                }
                
                ClauseSimp& operator*() {
                    return *it;
                }
                
                ClauseSimp*& operator->() {
                    return it;
                }
            private:
                ClauseSimp* it;
        };
        
        iterator begin()
        {
            return iterator(which.getData());
        }
        
        iterator end()
        {
            return iterator(which.getData() + which.size());
        }
};

#endif //CSET_H

