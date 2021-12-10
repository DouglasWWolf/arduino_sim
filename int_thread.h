#pragma once
#include <thread>


class InterruptThread
{
public:

    // Spawns the thread
    void    spawn();

protected:

    // When this thread spawns this starts executing
    void    main();

    // This is to allow the external "launch_thread()" function access to "main()"
    friend unsigned int launch_cthread(void*);
};

