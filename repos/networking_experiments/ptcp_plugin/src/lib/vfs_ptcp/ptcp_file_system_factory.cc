//
// Created by anton on 24.01.2022.
//

#include <vfs/file_system_factory.h>
#include <timer_session/connection.h>

extern "C" Vfs::File_system_factory *vfs_file_system_factory(void)
{
    struct Factory : Vfs::File_system_factory
    {
        Genode::Constructible<Timer::Connection> timer { };

        Vfs::File_system *create(Vfs::Env &vfs_env, Genode::Xml_node config) override
        {
            Genode::error("dummy create called");
            return nullptr;
        }
    };

    static Factory f;
    return &f;
}