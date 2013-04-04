/*
 * Copyright © 2013 Canonical Ltd.
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

#ifndef MIR_GRAPHICS_ANDROID_DEFAULT_FRAMEBUFFER_FACTORY_H_
#define MIR_GRAPHICS_ANDROID_DEFAULT_FRAMEBUFFER_FACTORY_H_

#include "framebuffer_factory.h"

namespace mir
{
namespace graphics
{
namespace android
{

class GraphicAllocAdaptor;

class DefaultFramebufferFactory : public FramebufferFactory
{
public:
    explicit DefaultFramebufferFactory(std::shared_ptr<GraphicAllocAdaptor> const& buffer_allocator);
    std::shared_ptr<ANativeWindow> create_fb_native_window(std::shared_ptr<DisplayInfoProvider> const&);
private:
    std::shared_ptr<GraphicAllocAdaptor> buffer_allocator;
};

}
}
}

#endif /* MIR_GRAPHICS_ANDROID_DEFAULT_FRAMEBUFFER_FACTORY_H_ */
