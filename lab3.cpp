#include "lab3.h"
#include <cstring>
#include <windows.h>
#include <vector>
#include <fstream>

using namespace std;

unsigned int lab3_thread_graph_id()
{
    return 6;
}

const char* lab3_unsynchronized_threads()
{
    return "deg";
}

const char* lab3_sequential_threads()
{
    return "bcd";
}

#define NUMBER_OF_THREADS int('m'-'a'+1)
#define MULTIPLIER 3
#define BORDERS_COUNT 6


// thread identifiers array
HANDLE tida[NUMBER_OF_THREADS];
HANDLE tid(char thread_name) {
    return tida[thread_name - 'a'];
}

HANDLE semaphores[NUMBER_OF_THREADS];

namespace my {
    HANDLE create_sem(int value = 0) {
        HANDLE result = CreateSemaphore(NULL, value, 1, NULL);
        return result;
    }

    vector<HANDLE> sems[BORDERS_COUNT];
    int threads_on_boarder[] = { 1,3,3,3,3,2 };
    int next_border = 0;


    HANDLE stdout_lock;
    HANDLE border_sync_lock;

    void sync_border(char name) {
        if (next_border == BORDERS_COUNT) return;

        HANDLE sem_for_this_thread = create_sem();

        WaitForSingleObject(stdout_lock, INFINITE);
        WaitForSingleObject(border_sync_lock, INFINITE);
        sems[next_border].push_back(sem_for_this_thread);
        // cerr << "\nborder " << next_border << " got +1 from " << name << ", now = " << sems[next_border].size() << endl;
        if (sems[next_border].size() == threads_on_boarder[next_border]) {
            // cout << "|" << flush;
            // cerr << "\nreleasing border " << next_border << endl;
            for (HANDLE s : sems[next_border]) ReleaseSemaphore(s, 1, NULL);
            ++next_border;
        }
        ReleaseMutex(stdout_lock);
        ReleaseMutex(border_sync_lock);

        WaitForSingleObject(sem_for_this_thread, INFINITE);
    }



    int computation_counts_per_thread[NUMBER_OF_THREADS];
    void compute(char name, bool sequentional = false) {
        WaitForSingleObject(stdout_lock, INFINITE);
        // ferr << name << flush;
        cout << name << flush;
        ReleaseMutex(stdout_lock);
        computation();
        if (!sequentional && (++computation_counts_per_thread[name - 'a']) % MULTIPLIER == 0) {
            sync_border(name);
        }
    }

    bool first_ever_sem = true;

    HANDLE obtain_sem(char name) {
        HANDLE result = semaphores[name - 'a'];
        if (!result) {
            result = semaphores[name - 'a'] = create_sem();
        }
        return result;
    }

    HANDLE fetch_next_sem(char name) {
        int i = name - 'a';
        do {
            i = (i + 1) % NUMBER_OF_THREADS;
        } while (!semaphores[i] && i != name - 'a');
        // return i == name-'a' ? NULL : semaphores[i];
        return semaphores[i];
    }

    void purge_sem(char name) {
        CloseHandle(semaphores[name - 'a']);
        //        delete semaphores[name-'a'];
        semaphores[name - 'a'] = NULL;
    }

    void seq_compute(char name, bool last) {
        WaitForSingleObject(obtain_sem(name), INFINITE);
        compute(name, true);
        if (last) purge_sem(name);
        HANDLE next_sem = fetch_next_sem(name);
        if (next_sem) ReleaseSemaphore(next_sem, 1, NULL);
        if ((++computation_counts_per_thread[name - 'a']) % MULTIPLIER == 0) {
            sync_border(name);
        }
    }

    void create_thread(LPTHREAD_START_ROUTINE entry, string thread_name) {
        HANDLE newThread = CreateThread(NULL, 0, entry, NULL, 0, NULL);
        tida[thread_name[0] - 'a'] = newThread;
        if (!newThread) cerr << "Can't create thread. Error: " << GetLastError() << endl;
    }
#define MY_THREAD_CREATE(name) my::create_thread(gen::thread_##name, #name)
}

#define REPEAT(cnt) for (int i = 0; i < cnt*MULTIPLIER; ++i)

namespace gen {
    DWORD WINAPI thread_a(LPVOID pointer);
    DWORD WINAPI thread_b(LPVOID pointer);
    DWORD WINAPI thread_c(LPVOID pointer);
    DWORD WINAPI thread_d(LPVOID pointer);
    DWORD WINAPI thread_e(LPVOID pointer);
    DWORD WINAPI thread_f(LPVOID pointer);
    DWORD WINAPI thread_g(LPVOID pointer);
    DWORD WINAPI thread_h(LPVOID pointer);
    DWORD WINAPI thread_i(LPVOID pointer);
    DWORD WINAPI thread_k(LPVOID pointer);
    DWORD WINAPI thread_m(LPVOID pointer);

    DWORD WINAPI thread_a(LPVOID pointer) {
        REPEAT(1) my::compute('a');

        MY_THREAD_CREATE(b);
        MY_THREAD_CREATE(d);
        MY_THREAD_CREATE(c);

        WaitForSingleObject(tid('c'), INFINITE);

        return 0;
    }

    DWORD WINAPI thread_b(LPVOID pointer) {
        REPEAT(1) my::seq_compute('b', i + 1 == 1 * MULTIPLIER);

        return 0;
    }

    DWORD WINAPI thread_c(LPVOID pointer) {
        REPEAT(1) my::seq_compute('c', i + 1 == 1 * MULTIPLIER);

        WaitForSingleObject(tid('b'), INFINITE);

        MY_THREAD_CREATE(e);
        MY_THREAD_CREATE(g);

        WaitForSingleObject(tid('e'), INFINITE);

        return 0;
    }

    DWORD WINAPI thread_d(LPVOID pointer) {
        REPEAT(2) my::seq_compute('d', i + 1 == 2 * MULTIPLIER);

        return 0;
    }

    DWORD WINAPI thread_e(LPVOID pointer) {
        REPEAT(1) my::compute('e');

        WaitForSingleObject(tid('d'), INFINITE);

        MY_THREAD_CREATE(f);
        MY_THREAD_CREATE(h);

        WaitForSingleObject(tid('f'), INFINITE);

        return 0;
    }

    DWORD WINAPI thread_f(LPVOID pointer) {
        REPEAT(1) my::compute('f');

        WaitForSingleObject(tid('g'), INFINITE);

        MY_THREAD_CREATE(i);
        MY_THREAD_CREATE(k);

        WaitForSingleObject(tid('i'), INFINITE);

        return 0;
    }

    DWORD WINAPI thread_g(LPVOID pointer) {
        REPEAT(2) my::compute('g');

        return 0;
    }

    DWORD WINAPI thread_h(LPVOID pointer) {
        REPEAT(2) my::compute('h');

        return 0;
    }

    DWORD WINAPI thread_i(LPVOID pointer) {
        REPEAT(1) my::compute('i');

        WaitForSingleObject(tid('h'), INFINITE);

        MY_THREAD_CREATE(m);

        WaitForSingleObject(tid('m'), INFINITE);

        return 0;
    }

    DWORD WINAPI thread_k(LPVOID pointer) {
        REPEAT(2) my::compute('k');

        return 0;
    }

    DWORD WINAPI thread_m(LPVOID pointer) {
        REPEAT(1) my::compute('m');

        return 0;
    }
}

int lab3_init()
{
    my::next_border = 0;

    my::stdout_lock = CreateMutex(NULL, FALSE, NULL);
    my::border_sync_lock = CreateMutex(NULL, FALSE, NULL);

    semaphores['c' - 'b'] = my::create_sem(1);

    MY_THREAD_CREATE(a);

    WaitForSingleObject(tid('a'), INFINITE);

    // free resources
    CloseHandle(my::stdout_lock);
    CloseHandle(my::border_sync_lock);

    for (int i = 0; i < BORDERS_COUNT; ++i) {
        for (HANDLE s : my::sems[i]) {
            CloseHandle(s);
            //delete s;
        }
        my::sems[i].clear();
    }

    // ferr.close();


    std::cout << std::endl;
    return 0;
}
