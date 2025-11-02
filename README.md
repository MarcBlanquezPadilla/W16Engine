# W16 ENGINE

### Short Description
**W16 ENGINE** is a 3D game engine developed in **C++** using **SDL3**, **OpenGL**, **Assimp**, **DevIL**, **Glad**, **ImGUI** and **GLM**. It features a modular architecture inspired by Unity.  
It allows loading 3D models and textures, managing hierarchies of GameObjects, and editing components through an in-engine editor.  
The engine includes a full suite of tools for interactive scene visualization and game development.

🔗 **GitHub Repository:** [https://github.com/MarcBlanquezPadilla/W16Engine]

---

##  Development Team
**W16 ENGINE** is powered by **Torrats Games**.
| Name | GitHub |
|------|--------|
|Marc Blánquez | https://github.com/MarcBlanquezPadilla |
| Martí Mira | https://github.com/algars15 |
| Arnau Balasch | https://github.com/Balar05 |

---

### Extra funtionalities
- **Functional transform** : Position, rotation and scale work in real time.

---

##  How to Use the Engine

### Running the Engine
1. Run the executable `W16Engine.exe`.  
2. The `Baker_house.fbx` model loads automatically on startup.  
3. You can **drag & drop** files directly into the window:
   - **FBX files** → Added as a new *GameObject* with its *Mesh* and *Texture*.  
   - **DDS/PNG files** → Applied as a texture to the currently selected *GameObject*.

---

### Camera Controls
| Action | Keys / Mouse |
|--------|---------------|
| Free look around | Hold **Right Mouse Button** and move the mouse |
| Free movement | Hold **Right Mouse Button** + **WASD** |
| Zoom | **Mouse wheel** |
| Orbit around object | **Alt + Left Click** |
| Focus selected GameObject | **F** |
| Double movement speed | Hold **Shift** while moving |

---

### Game Objects
| Action | Keys / Mouse |
|--------|---------------|
| Import model from PC | Drag and drop the FBX |
| Basic Shapes | Right clic in the **Hierarchy** |
| Load textures | Drag and drop the texture file while selecting the mesh |
| Toggle normals | Check the button in the inspector |
| Transform | Change parameters in the inspector |
| Use checkered texture | Toggle button in the inspector |

---

## Engine Structure

### GameObjects & Components
Each loaded model becomes a **GameObject** with the following components:
- **Transform** – position, rotation, and scale.  
- **Mesh** – geometry loaded via *Assimp*.  
- **Texture** – image loaded with *DevIL*.

### Editor Windows
- **Console**: Displays logs for geometry and texture loading, library initialization, app flow, and errors.  
- **Configuration**:  
  - Real-time FPS graph.  
  - Settings for all mod
  - Memory consume information
 
---

## Tool bar functions
- **File** : Exit the engine
- **View** : Toggle window visibility
- **Help** : Documentation, repository link, report issues and about.
