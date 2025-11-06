# Report Assignment 2 3DCGM
07/11/2025

Vlad Tudor Stefanescu - 5465877

Alexandru - Cristian Dumitrache - 5487773

## Introduction
This report presents the features and implementation details of the 3D Computer Graphics and Modelling (3DCGM) assignment 2. The project focuses on creating a 3D scene conainting a solar system and a spaceship that can navigate through it.

## Contributions
TODO

## Features Implemented
Before diving in, our project uses a toml config file which can be chosen before running the project. We have multiple of these files which are used for different things inside the configurations folder. Configurable parameters in the toml are written like this `toml:param_category.param_name`. ImGUI configurable parameter names are written like this `imgui:param_name`.

### Solar System
**Celestial Bodies**

The celestial bodies in our system use a runtime generated mesh of a class I geodesic icosahedron [https://en.wikipedia.org/wiki/Geodesic_polyhedron#Class_I].

This mesh is generated with a frequency (`toml:planets.ico_mesh_resolution`) and then used to render all celestial bodies in our project. The difference made by using a geodesic icosahedron instead of a regular icosahedron is in Figure 1 below.

| Regular Icosahedron          | Geodesic Icosahedron         |
|:-------------------:|:-------------------:|
| `geodesic_resolution_1.toml` | `geodesic_resolution_3.toml` |
| ![Regular Icosahedron](screenshots/regular_icosahedron.png) | ![Geodesic Icosahedron](screenshots/geodesic_icosahedron.png) |

*Figure 1: Comparison between a regular icosahedron and a class I geodesic icosahedron mesh with frequency 3 used for celestial bodies.*

We implement 4 types of celestial bodies (some of which we will go into more depth below):
- Default body (This is the parent class of all other body types and implements default behaviour. It is used as a placeholder for moons.)
- Star
- Earth
- Null body (not rendered, but used for orbital anchoring)

Default bodies also have basic attributes like position, color(`imgui:Fallback Color`, `toml:planets.fallback_color`), ka(`toml:planets.fallback_ka`), kd(`toml:planets.fallback_kd`), and radius. The radius can also be configured using imgui, or toml (we explain this below). n.b. The color, ka, and kd are not used if the body is a child class of default body.

**Tesselation**

All Cellestial bodies implement adaptive tesselation to improve the resolution of the geodesic icosahedron mesh.

This can be toggled(`imgui:Body Tesselation`, `toml:planets.enable_body_tesellation`), and can be made to adjust the height of triangles in pixels(`imgui:Target Body Tessellation Triangle Height`, `toml:target_triangle_height`).

The reason for adding gpu side tessellation alongside cpu side tesselation (the geodesic icosahedron) is because it is dynamic. However, the increase of resolution that can be achieved in opengl with tesselation is limited, so we still need cpu side tessellation. Another reason for using both is that with CPU tesselation we have more control over how triangles are split, resulting in a better distribution.

An alternative to this method would be to use a high triangle count mesh of a sphere but because of the way we implemented terrain (see below Earth section), tessellation is better suited to our needs.

Below is a table comparing (on a geodesic with frequency 1) tesselation off, tesselation with target triangle hegiht 100, and tesselation with target triangle height 1.

| Tessellation Off | Triangle Height 100 | Triangle Height 1 |
|:----------------:|:-------------------:|:-----------------:|
| `tessellation_off.toml` | `tessellation_on_100.toml` | `tessellation_on_1.toml` |
| ![Tessellation Off](screenshots/tessellation_off.png) | ![Triangle Height 100](screenshots/tessellation_100.png) | ![Triangle Height 1](screenshots/tessellation_1.png) |

*Figure 2: Comparison of tessellation settings on a geodesic icosahedron mesh with frequency 1.*

The tesselation evaluation step is basic, it just outputs worldspace position, normalized position on the sphere (i.e. model space position), and the clip space position. However, the earth implements a more advanced tesselation evaluation, which will be explained in the Earth section.

**Stars**

Stars implement color using temperature. This is configurable as a global default(`toml:planets.star.default_star_temperature`) and on a per star basis(`imgui:Temperature (C)`). Cold stars are red and the hottest are light blue. This temperature factor also impacts their light intensity.

![Star Color Comparison](screenshots/star_color_comparison.png)

*Figure 3: Star color comparison based on temperature in Celsius, from left to right 3000, 4500, 5778 (Sun), 10000 and 30000.* 

**Stars: Animated textures**

Stars also implement animated textures(`toml:planets.star.enable_star_texture`, `imgui: Enable Star Texture / Animation`). These textures are implemented using 4d noise in the fragment shader. The base opensimplex noise is taken from https://github.com/stegu/webgl-noise. Using this we implemented a fractal noise algorithm, which is used in conjunction with domain warping to get swirls. this value is then used to interpolate colors (based on temperature) to get a nice looking star.

The 4 dimensions are needed because we are sampling the noise at the fragPosition(3D) at the point in time t, our 4th dimension. Parameters for the noise function can be configured(`toml:planets.star`).

![Star Texture Animation](screenshots/star_textured.png)

*Figure 4: Star with animated textures enabled. To see the effect run the project, or look at the demo.*

**Earth Terrain**

The earth body type implements a procedural terrain shader (`imgui:Enable Planet Terrain Generation`). This shader uses 3D opensimplex noise (same as the star, but 3D) to displace the vertices of the earth in the tesselation evaluation shader. The noise parameters can be configured(`toml:planets.earth`). This noise value is also enhanced by implementing a fractal noise algorithm. It is then multiplied by two and raised to the power of 1.5 (while preserving sign) to get more pronounced terrain features. This value is then compared to the ocean level(`imgui: Ocean Level`, `toml:planets.earth.ocean_level`) to determine if the vertex is underwater or above water. If it is underwater, the vertices are displaced on the sphere of radius equivalent to the ocean level, otherwise they are displaced on the sphere of radius equal to the ocean level plus the noise displacement.

| Terrain Off | Terrain On(no tessellation) | Terrain On(tessellation) |
|:----------------:|:-------------------:|:-----------------:|
| ![Terrain Off](screenshots/earth_no_generation.png) | ![Terrain On no tessellation](screenshots/earth_no_tessellation_generated.png)  | ![Terrain On with tessellation](screenshots/earth_tessellation_generated.png) |

*Figure 5: Comparison of earth terrain generation settings. This displays the benefit of using tesselation as well.*

The color of the surface is alos determined by the height. Transitoning from beach to grass, forest and snow.

**Earth Surface Normals**

The earth fragment shader also implements a noise based surface normal map(`imgui:Enable Planet Surface Normals`). This is done by sampling the same fractal 3D noise function as the above, and using the derivatives of this noise to perturb the normal. The parameters for this noise can also be configured(`toml:planets.earth.shape_noise_*`).

This normal is also used to use gray color for cliffs, i.e. where the slope is steep.
| Surface Normals Off | Surface Normals On |
|:----------------:|:-------------------:|
| ![Surface Normals Off](screenshots/earth_no_surface_normals.png) | ![Surface Normals On](screenshots/earth_surface_normals.png)  |

*Figure 6: Comparison of earth surface normal settings. Note the added detail on the terrain when normals are enabled.*


**Earth Water**

The earth fragment shader also implements a noise based water normal map(`imgui:Enable Planet Water Normals`, `toml:`). This is done by sampling another fractal 4D noise function, and using the derivatives of this noise to perturb the normal. The parameters for this noise can also be configured(`toml:planets.earth.water_noise_*`).

| Water Normals Off | Water Normals On |
|:----------------:|:-------------------:|
| ![Water Normals Off](screenshots/earth_no_water_normals.png) | ![Water Normals On](screenshots/earth_with_water_normals.png)  |

*Figure 7: Comparison of earth water normal settings. Note the added detail on the water surface when normals are enabled. Note: the mountains peeking through are to be ignored, because, in order to take these screenshots, I increased the ocean level.*

The water also has color based on the depth of the ocean at that point. The deeper the water, the darker the blue, and the shore is white to represent foam.

**Note:** these noise functions are all computed in the shaders, no precomputed textures are used. This allows for infinite resolution and seamless texturing, however it is more computationally expensive.

**Hierarchical transformations**

Celestial bodies are stored in a list, but each body also has a pointer to its parent body (if it has one). This allows us to implement hierarchical transformations, where a body's position is relative to its parent. For example, a moon's position is relative to the planet it orbits. 

For simplicity, we do not update the bodies using a topological sort, instead we just update them in the order they are stored in the list. This means that there is a lag in the position update of child bodies, but this is negligible for our purposes.

The orbits are eliptical, with the parent being at one of the foci. The orbital parameters (major radius(`imgui: large radius`), minor radius(`imgui: small radius`), direction (`imgui: orbit direction`) and normal(of the orbital plane)(`imgui: orbit normal`) are configurable.

The whole solar system can be set up in the toml config file(`toml:planets.planets_info`), where each body has a type, parent, and optional orbit parameters like in the example below:
```toml
[planets]
planets_info = [
    # example entries:
    # [type, radius, parent_id = -1, ...] -> no orbit, static planet at (0, 0, 0) OR
    # [type, radius, parent_id, orbit_direction, orbit_small_r, orbit_large_r, orbit_normal, orbit_period]
    # type can be "star", "earth", "null" or "body"

    # Default single planet system
    ["star", 4.0, -1, [0.0, 0.0, 0.0], 0.0, 0.0, [0.0, 1.0, 0.0], 0.0],
    ["earth", 1.0, 0, [1.0, 0.0, 0.0], 40.0, 40.0, [0.0, 1.0, 0.0], 100000.0],
    ["body", 0.5, 1, [0.0, 0.0, 1.0], 10.0, 10.0, [0.0, 1.0, 0.0], 2.0],
]
```

We also have the option of creating binary star systems using null bodies as orbital anchors. The binary flag(`toml:planets.binary_system`) is used for shadow and lighting calulations (explained below). Otherwise, only the first star in the bodies list is considered for lighting calculations, but many stars can be rendered. The null body type is used for orbital anchoring, it is not rendered, but can be used as a parent for other bodies. This is useful for creating binary star systems where the stars orbit a null body.

### Lighting and Shading

**Eclipses**

We implement eclipses for both single and binary star systems(`imgui:enable eclipses`). Celestial bodies can cast shadows on each other and the ship. This is done by comparing the apparent sizes and angular separations of celestial bodies as seen from a given point. Each body and the light source (the sun) are projected as discs on the sky, with angular radii determined by their physical size and distance. By evaluating how much these discs overlap, the algorithm estimates the fraction of the sun’s disc that is obscured, yielding a continuous eclipse factor between zero and one. It assumes simple geometric occlusion without detailed umbra or penumbra modeling, combining multiple overlaps multiplicatively to approximate partial and total eclipses. This is done in the fragment shader for both celestial bodies and the ship.

| No Eclipse | Eclipse | Eclipse + Earth |
|:----------------:|:-------------------:|:-----------------:|
| ![No Eclipse](screenshots/no_eclipse.png) | ![Eclipse](screenshots/eclipse.png)  | ![Eclipse Earth](screenshots/eclipse_generated.png) |

*Figure 8: Comparison of eclipse settings.*

**Star lights (shadow mapping)**

For lighting, each star acts as a point light source. Shadows are handled using shadow mapping(`imgui: Enable Shadowmapping`) and are generated only for earths and the ship. We generate one shadow map per star and earth/ship combination. These shadow maps are only meant to capture shadows cast by an object onto itself, so the shadow map frustum is tightly fitted around the object. This is done by computing a bounding orthographic projection around the object in the light's view space, and using this to set the near and far planes of the shadow map frustum.

| Shadowmapping Off Earth | Shadowmapping On Earth | Shadowmapping Off Ship | Shadowmapping On Ship |
|:----------------:|:-------------------:|:-----------------:|:-----------------:|
| ![Shadowmapping Off Earth](screenshots/earth_no_shadow.png) | ![Shadowmapping On Earth](screenshots/earth_shadow.png)  | ![Shadowmapping Off Ship](screenshots/ship_no_shadow.png) | ![Shadowmapping On Ship](screenshots/ship_shadow.png) |

Figure 9: Comparison of shadowmapping settings on earth and ship.

The sampling of the shadow map is done using Percentage Closer Filtering (PCF) to smooth out the shadows(`imgui: PCF Kernel Radius`, `imgui:Enable PCF`).

**Cellestial Body Shading**

The earths' surface and default bodies use lambertian diffuse shading. The earths' water uses blinn-phong shading for the specular component.

**Light Combination**
TODO:

**PBR Shading for the Ship**
TODO: