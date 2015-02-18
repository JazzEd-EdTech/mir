/*
 * Copyright © 2014 Canonical Ltd.
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
 * Authored by: Daniel van Vugt <daniel.van.vugt@canonical.com>
 */

#ifndef MIR_EXAMPLES_DEMO_RENDERER_H_
#define MIR_EXAMPLES_DEMO_RENDERER_H_

#include "mir/compositor/gl_renderer.h"
#include "stub_gl_text_cache.h"

#include <unordered_set>

namespace mir
{
namespace examples
{

enum ColourEffect
{
    none,
    inverse,
    contrast,
    neffects
};

class DemoRenderer : public compositor::GLRenderer
{
public:
    DemoRenderer(
        geometry::Rectangle const& display_area,
        compositor::DestinationAlpha dest_alpha,
        float const titlebar_height,
        float const shadow_radius);
    ~DemoRenderer();

    void begin(std::unordered_set<graphics::Renderable::ID> renderables_not_to_decorate) const;
    
    bool would_embellish(
        graphics::Renderable const& renderable,
        geometry::Rectangle const&) const;

    void set_colour_effect(ColourEffect);

protected:
    void tessellate(std::vector<graphics::GLPrimitive>& primitives,
                    graphics::Renderable const& renderable) const override;

    void draw(graphics::Renderable const& renderable,
              GLRenderer::Program const& prog) const override;

private:
    void tessellate_shadow(
        std::vector<graphics::GLPrimitive>& primitives,
        graphics::Renderable const& renderable,
        float radius) const;
    void tessellate_frame(
        std::vector<graphics::GLPrimitive>& primitives,
        graphics::Renderable const& renderable,
        float titlebar_height) const;

    float const titlebar_height;
    float const shadow_radius;
    float const corner_radius;
    GLuint shadow_corner_tex;
    GLuint titlebar_corner_tex;

    ColourEffect colour_effect;
    Program inverse_program, contrast_program;
    
    mutable std::unordered_set<graphics::Renderable::ID> decoration_skip_list;

    mutable StubGLTextCache text_cache;
};

} // namespace examples
} // namespace mir

#endif // MIR_EXAMPLES_DEMO_RENDERER_H_
