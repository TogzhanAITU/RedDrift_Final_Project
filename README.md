```markdown
# RedDrift

A 2D side-scrolling space shooter written in C++17 and OpenGL 3.3. Fly your cruiser from Earth to Mars, dodge asteroids, fight enemy ships, and spend earned Sol on upgrades in the workshop.

Built for the Computer Graphics Fundamentals course at Astana IT University (Group: IT-2504 | Instructor: Sabina Saleshova).

## Procedural Graphics

Everything on screen (ships, planets, space stations, explosions, UI, and fonts) is drawn procedurally from triangles in C++.
- No external textures, 3D models, or font files are loaded from disk.
- Visuals rely on GLSL 330 shaders, procedural geometry, and alpha blending.
- The renderer is strictly 2D (no 3D camera, lighting, or depth testing).

## Visual Style

The game uses a military sci-fi palette with dark armored hulls, glowing thrusters, and warm sunset tones for Mars. Reference images were used only for silhouette and color inspiration; all geometry and rendering code are written from scratch.

## Quick Start

### Prerequisites
- CMake 3.16+
- C++17 compiler (Visual Studio 2022, GCC, or Clang)
- GPU driver supporting OpenGL 3.3 Core

GLFW, GLEW, and GLM are bundled in `vendor/`.

### Build and Run

**Windows (Visual Studio)**
```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release
.\build\Release\RedDrift.exe

```

**Linux**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/RedDrift

```

## Controls

| Action | Key  |  Input | Description |
| ---  | --- | --- |
| Move  | WASD / Arrow Keys | Fly inside the flight corridor |
| Pulse Fire | Space / Left Click | Fire standard bullets |
| Continuous Laser | Right Mouse Button (Hold) | Heavy beam weapon (unlock in workshop) |
| Homing Missiles | E (Hold) | Lock on and launch missiles (unlock in workshop) |
| Boost | Shift | Speed up using the shared energy reserve |
| Pause | P / Esc | Pause/resume mission or exit menu |
| Retry | R | Restart current stage |
| Menu / Back | M / Esc | Return to level select |
| Fullscreen | F11 | Toggle windowed and fullscreen mode |
| Level Select | 1-6, Mouse, Enter | Choose and start a stage |
| Workshop | W, Tab, Arrows, 1-2 | Open upgrade workshop |

## Features

* **6 Stages**: Progress from Earth's stratosphere past the Moon and through deep space to Mars, including launch, Moon landing/take-off, and finale sequences.
* **Economy and Upgrades**: Earn Sol by destroying enemies (25 per light ship, 60 per heavy ship). Upgrade hull and energy capacity (up to 225) and unlock laser and homing missiles (levels 1-5).
* **Local Saves**: Sol, unlocked weapons, and stage progress are saved automatically (`%LOCALAPPDATA%\RedDrift\progress.txt` on Windows).
* **Automated Tests**: Four CTest suites verify flight, workshop logic, weapons, and the Mars finale:
```bash
ctest --test-dir build -C Release --output-on-failure

```



## Project Structure

* `src/main.cpp`: Window setup, input handling, main loop, and smoke tests.
* `src/Level.*`, `src/LevelWeapons.cpp`: Stage logic, enemy waves, collisions, and weapons.
* `src/Player.*`, `src/Progress.*`: Ship movement, energy management, and save persistence.
* `src/Renderer*.cpp`: Procedural geometry generation for ships, scenes, effects, and UI.
* `shaders/`: GLSL 330 vertex and fragment shaders.
* `tests/`: Automated CTest programs.

## Credits

* **Togzhan Nurtaza**: Design, report, and presentation
* **Yerassyl Kenzhe**: Rendering
* **Adilzhan Alimzhan**: Software and game logic

### Third-Party Libraries

* GLFW (zlib/libpng license)
* GLEW (BSD-style license)
* GLM (MIT license)

License files are included in `vendor/`.

```

```
