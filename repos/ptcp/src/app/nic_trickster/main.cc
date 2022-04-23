/*
 * \brief  Bump-in-the-wire component to dump NIC traffic info to the log
 * \author Martin Stein
 * \date   2017-03-08
 */

/*
 * Copyright (C) 2016-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode */
#include <base/component.h>
#include <base/heap.h>
#include <base/attached_rom_dataspace.h>
#include <timer_session/connection.h>

/* local includes */
#include "component.h"
#include <nic_trickster/control/stopper.h>

using namespace Net;
using namespace Genode;


class Main {
private:

    Attached_rom_dataspace _config;
    Timer::Connection _timer;
    Duration _curr_time{Microseconds(0)};
    Heap _heap;
    Net::Root _root;
    Stopper_root _stopper;

public:

    Main(Env &env);
};


Main::Main(Env &env)
        :
        _config(env, "config"), _timer(env), _heap(&env.ram(), &env.rm()),
        _root(env, _heap, _config.xml(), _timer, _curr_time),
        _stopper(env.ep(), _heap) {
    env.exec_static_constructors();
    env.parent().announce(env.ep().manage(_root));
    env.parent().announce(env.ep().manage(_stopper));
}


void Component::construct(Env &env) { static Main main(env); }
