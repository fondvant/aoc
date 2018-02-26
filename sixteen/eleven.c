#define floors 4
#define absolute_max_attempts 70

#ifdef EXAMPLE
#define e_len 4
#define starting_e 0, 1, 0, 2
#else
#define e_len 14
#define starting_e 0, 0, 1, 0, 1, 0, 2, 2, 2, 2, 0, 0, 0, 0
#endif

#include <stdio.h>
#include <stdlib.h>

struct state {
    int elements[e_len];
    int vator;
    int uniqel;
    struct state *prev;
};

struct attempt { // uniq + lowest possible # of attempts
    int u;
    int a;
};

struct attempt alluniqs[123456789];
int uniqindex;

int max_attempts;
int total_attempts, total_identical, total_disregard, total_dead, total_giveup;

int attempt(struct state *s, int elevator, int a, int b, int attempts) {

    total_attempts++;

    // pre move optimisations!

    // disregard attempts at M+G unless they're the same type
    // M+G means (alice+bob)%2==1, but
    // if they ARE the same type, alice%2==0 and bob is one more
    if((a + b) % 2)
        if(!(a % 2 == 0 && b == a + 1)) {
            total_disregard++;
            return 0xbeef;
        }

    // create new floor
    struct state ns;
    ns.prev = s;
    ns.vator = elevator;
    int i;
    for(i=0; i < e_len; i++) {
        if(i == a || i == b)
            ns.elements[i] = ns.vator;
        else
            ns.elements[i] = s->elements[i];
    }
    ns.uniqel = getuniqel(ns.elements, ns.vator);
    //printf("unique id: 0x%x\n", ns.uniqel);

    // post move optimisations

    // disregard all unnecessary moves

    // don't go back to where you were ever
    int is_unique = 1;
    for(i=0; i < uniqindex; i++)
        if(ns.uniqel == alluniqs[i].u) {
            if(attempts < alluniqs[i].a) { // ... unless this was faster
                //printf("went from %d to %d using this path:\n", alluniqs[i].a, attempts);
                alluniqs[i].a = attempts;
                //printpath(&ns);
                is_unique = 0;
                break;
            }
            else { // otherwise give up
                total_identical++;
                return 0xbeef;
            }
        }

    if(is_unique) {
        //printf("creating uniq 0x%x after %d attempts.\n", ns.uniqel, attempts);
        alluniqs[uniqindex].u = ns.uniqel;
        alluniqs[uniqindex++].a = attempts;
    }


    /*
    for(i=0; i < uniqindex; i++)
        printf("entry %3d: 0x%x\n", i, alluniqs[i]);
    */

    // check if won
    int won = 1;
    for(i=0; i < e_len; i++)
        won &= ns.elements[i] == floors - 1;
    if(won) {
        printf("won after %d attempts :D and %d total\n", attempts, total_attempts);
        printpath(&ns);
        return 0;
    }

    // check if dead
    int alive = 1;
    for(i=0; i < e_len; i += 2) { // for each Microchip (may optimise)
        if(ns.elements[i] == ns.elements[i+1]) // same as Gen, always safe
            ;
        else {
            int j;
            for(j=1; j < e_len; j += 2) { // for each generator
                if(j != i+1) { // except the corresponding one
                    alive &= (ns.elements[i] != ns.elements[j]); // ensure no match
                }
            }
        }
    }
    if(!alive) { // dead, stop
        total_dead++;
        return 666;
    }

    else if(attempts >= max_attempts) { // give up
        total_giveup++;
        return 1000 + attempts;
    }

    else { // alright, go for it. no, please.

        // find movable dudes
        int num_cand = 0;
        int candidates[e_len];
        int i;
        for(i = 0; i < e_len; i++)
            if(ns.elements[i] == ns.vator)
                candidates[num_cand++] = i;

        int alice, bob, retval;

        // try moving two dudes up
        if(ns.vator != floors - 1) {
            if(num_cand >= 2) {
                // try all combos of two
                // lowest index will be first always. useful?
                for(alice=0; alice < num_cand - 1; alice++) {
                    for(bob = alice + 1; bob < num_cand; bob++) {
                        // alright, let's try
                        retval = attempt(&ns, ns.vator+1, candidates[alice], candidates[bob], attempts+1);
                        if(!retval) { // success!
                            return retval;
                        }
                    }
                }
            }
        }

        // and one dude up
        if(ns.vator != floors - 1) {
            for(alice=0; alice < num_cand; alice++) {
                retval = attempt(&ns, ns.vator+1, candidates[alice], candidates[alice], attempts+1);
                if(!retval) { // success!
                    return retval;
                }
            }
        }

        // and one down
        if(ns.vator != 0) {
            for(alice=0; alice < num_cand; alice++) {
                retval = attempt(&ns, ns.vator-1, candidates[alice], candidates[alice], attempts+1);
                if(!retval) { // success!
                    return retval;
                }
            }
        }

        // try moving two dudes down
        if(ns.vator != 0) {
            if(num_cand >= 2) {
                // as above, several 'FIXME's
                for(alice=0; alice < num_cand - 1; alice++) {
                    for(bob = alice + 1; bob < num_cand; bob++) {
                        retval = attempt(&ns, ns.vator-1, candidates[alice], candidates[bob], attempts+1);
                        if(!retval) { // success!
                            return retval;
                        }
                    }
                }
            }
        }

        // we have failed
        return -1;
    }
}

int compare_ints(const void *a, const void *b) { // why do i need to write this
    const int *da = (const int *) a;
    const int *db = (const int *) b;
    return (*da > *db) - (*da < *db);
}

/**
  * returns unique identifier for a state
  * ignores whether m/g-pair is fooium or barium
  */
int getuniqel(int *e, int v) {
    int half = e_len / 2;
    int sorted[half];
    int i;
    for(i=0; i < half; i++) // first give m+g-state unique 4bit int
        sorted[i] = e[i*2] + (e[i*2+1] << 2); // FIXME: will break if floors > 4.
                                          // come to think of it, that is
                                          // probably what part 2 will be
    qsort(sorted, half, sizeof(int), compare_ints); // sort in whatever consistent order
    int ret = 0;
    for(i=0; i < half; i++) // merge into single int because we only need 8 / 20 bits
        ret += sorted[i] << (4 * i);
    ret += v << (half * 4); // throw in the elevator too
    return ret;
}

int printstate(int *f) {
    int i;
    for(i=0; i < e_len; i++) {
        printf("%d ", f[i]);
    }
    printf("\n");
}

int printpath(struct state *s) {
    printf("printing path...\n");
    while(s) {
        int i;
        for(i=0; i < e_len; i++) {
            printf("%d ", s->elements[i]);
        }
        printf(" v=%d\n", s->vator);
        s = s->prev;
    }
}

int main() {

    struct state orig_state = { // lol dangerous
        {starting_e},
        0, // starting elevator
        0, // unique id, important
        0}; // null pointer

    for(max_attempts=0; max_attempts <= absolute_max_attempts; max_attempts++) {
        // start fresh
        int i;
        for(i=0; i < uniqindex; i++) {
            alluniqs[i].u = 0;
            alluniqs[i].a = 0;
        }
        uniqindex = 0;
        // ok
        printf("trying up to %d attempts... so far a total of %d\n", max_attempts, total_attempts);
        if(!attempt(&orig_state, 0, 0, 0, 0)) // i guess even the last variable is unnecessary
            break;
    }
    printf("stopped after a total of %d\n%d identical\n%d N+G\n%d deaths\nand %d gave up due to tiredness\n", total_attempts, total_identical, total_disregard, total_dead, total_giveup);
}
