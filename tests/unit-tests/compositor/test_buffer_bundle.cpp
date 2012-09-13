/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "mir_test/mock_buffer.h"

#include "mir/geometry/dimensions.h"
#include "mir/compositor/buffer.h"
#include "mir/compositor/buffer_bundle.h"
#include "mir/compositor/buffer_swapper_double.h"

#include <mir_test/gmock_fixes.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace mc = mir::compositor;
namespace geom = mir::geometry;

namespace
{
struct MockSwapper : public mc::BufferSwapper
{
public:
    MockSwapper() {};
    MockSwapper(std::shared_ptr<mc::Buffer> buffer)
        : default_buffer(buffer)
    {
        using namespace testing;

        ON_CALL(*this, compositor_acquire())
        .WillByDefault(Return(default_buffer.get()));
        ON_CALL(*this, client_acquire())
        .WillByDefault(Return(default_buffer.get()));
    };

    MOCK_METHOD0(client_acquire,   mc::Buffer*(void));
    MOCK_METHOD1(client_release, void(mc::Buffer*));
    MOCK_METHOD0(compositor_acquire,  mc::Buffer*(void));
    MOCK_METHOD1(compositor_release,   void(mc::Buffer*));

private:
    std::shared_ptr<mc::Buffer> default_buffer;
};
}

class BufferBundleTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        width = geom::Width{1024};
        height =geom::Height{768};
        stride = geom::Stride{1024};
        pixel_format = mc::PixelFormat{mc::PixelFormat::rgba_8888};

        mock_buffer = std::make_shared<mc::MockBuffer>(width, height, stride, pixel_format);
        mock_swapper = std::unique_ptr<MockSwapper>(new MockSwapper(mock_buffer));
    }

    std::shared_ptr<mc::MockBuffer> mock_buffer;
    std::unique_ptr<MockSwapper> mock_swapper;
    geom::Width width;
    geom::Height height;
    geom::Stride stride;
    mc::PixelFormat pixel_format;
};

TEST_F(BufferBundleTest, get_buffer_for_compositor_binds)
{
    EXPECT_CALL(*mock_buffer, bind_to_texture())
    .Times(1);
    mc::BufferBundle buffer_bundle(std::move(mock_swapper));

    auto texture = buffer_bundle.lock_and_bind_back_buffer();
}

TEST_F(BufferBundleTest, get_buffer_for_compositor_handles_resources)
{
    using namespace testing;

    EXPECT_CALL(*mock_swapper, compositor_acquire())
    .Times(1);
    EXPECT_CALL(*mock_swapper, compositor_release(_))
    .Times(1);

    mc::BufferBundle buffer_bundle(std::move(mock_swapper));

    auto texture = buffer_bundle.lock_and_bind_back_buffer();
}

TEST_F(BufferBundleTest, get_buffer_for_client_releases_resources)
{
    using namespace testing;

    EXPECT_CALL(*mock_swapper, client_acquire())
    .Times(1);
    EXPECT_CALL(*mock_swapper, client_release(_))
    .Times(1);
    mc::BufferBundle buffer_bundle(std::move(mock_swapper));

    auto buffer_resource = buffer_bundle.secure_client_buffer();
}

TEST_F(BufferBundleTest, client_requesting_resource_queries_for_ipc_package)
{
    EXPECT_CALL(*mock_buffer, get_ipc_package())
    .Times(1);

    mc::BufferBundle buffer_bundle(std::move(mock_swapper));

    auto buffer_package = buffer_bundle.secure_client_buffer();
}

TEST_F(BufferBundleTest, client_requesting_package_gets_buffers_package)
{
    using namespace testing;

    std::shared_ptr<mc::BufferIPCPackage> dummy_ipc_package = std::make_shared<mc::BufferIPCPackage>();
    EXPECT_CALL(*mock_buffer, get_ipc_package())
    .Times(1)
    .WillOnce(Return(dummy_ipc_package));
    mc::BufferBundle buffer_bundle(std::move(mock_swapper));

    std::shared_ptr<mc::GraphicBufferClientResource> buffer_resource = buffer_bundle.secure_client_buffer();
    std::shared_ptr<mc::BufferIPCPackage> buffer_package = buffer_resource->ipc_package;
    EXPECT_EQ(buffer_package, dummy_ipc_package);
}

