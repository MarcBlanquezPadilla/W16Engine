# W16 ENGINE

W16 ENGINE is a custom 3D game engine developed in C++ from scratch. It features a modular architecture inspired by Unity and includes a full suite of tools for interactive scene visualization, resource management, and game development.

**Powered by:** Torrats Games

## Technology Stack

Built using industry-standard libraries:
* **Core:** SDL3
* **Graphics & Math:** Glad, GLM
* **Asset Management:** Assimp (Models), DevIL (Textures)
* **UI:** ImGUI, ImGuizmo

## Development Team

| Name | GitHub Profile |
| :--- | :--- |
| **Marc Blánquez** | [GitHub Profile](https://github.com/MarcBlanquezPadilla) |
| **Martí Mira** | [GitHub Profile](https://github.com/algars15) |
| **Arnau Balasch** | [GitHub Profile](https://github.com/Balar05) |

**Repository:** [W16 Engine on GitHub](https://github.com/MarcBlanquezPadilla/W16Engine)

---

## Key Features

### Core Systems
* **Modular Architecture:** Hierarchy-based system managing GameObjects and Components.
* **Event System:** Robust internal messaging system for engine events.
* **Time Management:** Precise delta-time calculation and game clock management.
* **Scene Serialization:** Full support to Save and Load scenes, preserving the state of all GameObjects.

### Rendering & Optimization
* **Spatial Partitioning:** Implemented Octree for optimized spatial queries and rendering.
* **Frustum Culling:** Performance optimization to only render objects within the camera's view.
* **Advanced Materials:** Support for transparent textures and stencil buffer operations.
* **Multiple Cameras:** Support for switching between Editor Camera and Game Camera components.

### Asset Management
* **Smart Resource System:** Custom .meta file generation for asset tracking and file hashing.
* **Project Window:** Integrated file browser to manage assets directly within the editor.
* **3D Asset Loading:** Support for FBX models (via Assimp) and textures (PNG/DDS via DevIL).

### Editor Tools
* **Mouse Picking:** Select objects directly in the 3D scene using raycasting.
* **Multi-Selection:** Select multiple objects simultaneously via standard input shortcuts.
* **Guizmos:** Integrated translation, rotation, and scaling tools directly in the scene view.

---

## How to Use

### Running the Engine
1. Run the executable W16Engine.exe.
2. The engine loads with a default scene or the last saved state.

### Controls & Interaction

#### General Interaction
| Action | Input / Method |
| :--- | :--- |
| **Select Object** | Left Click (Mouse Picking) |
| **Multi-Selection** | Hold Ctrl + Click |
| **Gizmo Operation** | Use the visual gizmos to Translate, Rotate, or Scale |
| **Orbit Object** | Alt + Left Click |
| **Focus Object** | Select GameObject + F |

#### Camera Controls
| Action | Input (Mouse/Keyboard) |
| :--- | :--- |
| **Free Look** | Hold Right Click + Move Mouse |
| **Free Movement** | Hold Right Click + W, A, S, D |
| **Zoom** | Mouse Wheel |
| **Sprint** | Hold Shift while moving |

#### GameObject Management
| Action | Method |
| :--- | :--- |
| **Import Model** | Drag & drop .fbx file from Project/PC to Scene |
| **Create Basic Shape** | Right-click in the Hierarchy window |
| **Apply Texture** | Drag & drop texture file onto a selected Mesh |
| **Parenting** | Drag & drop GameObjects within the Hierarchy |
| **Modify Transform** | Change values manually in the Inspector |
| **Toggle Normals** | Check the debug box in the Inspector |
| **Checkered Texture** | Toggle the debug button in the Inspector |

---

## Engine Structure

### GameObjects & Components
Every entity in the scene is a GameObject containing the following components:
* **Transform:** Handles position, rotation, and scale logic (supports parent-child hierarchy).
* **Mesh:** Geometry data loaded via Assimp.
* **Texture:** Material data loaded via DevIL.
* **Camera:** Defines the viewport and projection settings for rendering.

### Editor Windows
* **Scene View:** The main 3D workspace with debug drawing (AABB, Octree debugs).
* **Game View:** Represents what the player sees through the active game camera.
* **Project:** A file explorer for the Assets folder, allowing file manipulation and import management.
* **Inspector:** Real-time modification of component values and resource reassignment.
* **Hierarchy:** Tree view of the scene objects, supporting parenting and reordering.
* **Console:** Displays logs, errors, and system status.
* **Configuration:** Shows real-time FPS graph, memory consumption, and settings.
