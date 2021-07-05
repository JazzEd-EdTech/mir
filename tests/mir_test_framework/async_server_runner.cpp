/*
 * Copyright © 2014-2017 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 or 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored By: Alan Griffiths <alan@octopull.co.uk>
 */

#include "mir_test_framework/async_server_runner.h"
#include "mir_test_framework/command_line_server_configuration.h"
#include "mir_test_framework/canonical_window_manager_policy.h"

#include "mir/fd.h"
#include "mir/main_loop.h"
#include "mir/geometry/rectangle.h"
#include "mir/options/configuration.h"
#include "mir/options/option.h"
#include <mir/report_exception.h>
#include <mir/thread_name.h>
#include "mir/test/doubles/null_logger.h"
#include <miral/set_window_management_policy.h>
#include <mir/shell/canonical_window_manager.h>

#include <boost/throw_exception.hpp>

#include <gtest/gtest.h>

namespace geom = mir::geometry;
namespace ml = mir::logging;
namespace mo = mir::options;
namespace msh = mir::shell;
namespace mtd = mir::test::doubles;
namespace mtf = mir_test_framework;
namespace mt = mir::test;

namespace
{
std::chrono::seconds const timeout{20};
}

mtf::AsyncServerRunner::AsyncServerRunner() :
    set_window_management_policy{[](auto&){}}
{
    unsetenv("WAYLAND_DISPLAY");    // We don't want to conflict with any existing Wayland server
    configure_from_commandline(server);

    server.add_configuration_option(mtd::logging_opt, mtd::logging_descr, false);
    server.override_the_logger([&]()
        {
            std::shared_ptr<ml::Logger> result{};

            if (!server.get_options()->get<bool>(mtd::logging_opt))
                result = std::make_shared<mtd::NullLogger>();

            return result;
        });
    // TODO This is here to support tests that rely on the legacy window management code
    // once they go, then we can set set_window_management_policy appropriately, kill this
    // and remove msh::CanonicalWindowManager
    server.override_the_window_manager_builder([this](msh::FocusController* focus_controller)
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            return std::make_shared<msh::CanonicalWindowManager>(
                    focus_controller,
                    server.the_shell_display_layout());
#pragma GCC diagnostic pop
        });
}

void mtf::AsyncServerRunner::add_to_environment(char const* key, char const* value)
{
    env.emplace_back(key, value);
}

void mtf::AsyncServerRunner::start_server()
{
    set_window_management_policy(server);

    server.add_init_callback([&]
        {
            auto const main_loop = server.the_main_loop();
            // By enqueuing the notification code in the main loop, we are
            // ensuring that the server has really and fully started before
            // leaving start_mir_server().
            main_loop->enqueue(
                this,
                [&]
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    server_running = true;
                    started.notify_one();
                });
        });

    server.apply_settings();

    mt::AutoJoinThread t([&]
        {
            try
            {
                mir::set_thread_name("mtf/AsyncServer");
                server.run();
            }
            catch (std::exception const& e)
            {
                std::ostringstream error;
                mir::report_exception(error);
                FAIL() << error.str();
            }
            std::lock_guard<std::mutex> lock(mutex);
            server_running = false;
            started.notify_one();
        });

    std::unique_lock<std::mutex> lock(mutex);
    started.wait_for(lock, timeout, [&] { return server_running; });

    if (!server_running)
    {
        BOOST_THROW_EXCEPTION(std::runtime_error{"Failed to start server thread"});
    }
    server_thread = std::move(t);
}

void mtf::AsyncServerRunner::stop_server()
{
    server.stop();
    wait_for_server_exit();
}

void mtf::AsyncServerRunner::wait_for_server_exit()
{
    std::unique_lock<std::mutex> lock(mutex);
    started.wait_for(lock, timeout, [&] { return !server_running; });

    if (server_running)
    {
        BOOST_THROW_EXCEPTION(std::logic_error{"stop_server() failed to stop server"});
    }
    server_thread.stop();
}

mtf::AsyncServerRunner::~AsyncServerRunner() noexcept
{
}
