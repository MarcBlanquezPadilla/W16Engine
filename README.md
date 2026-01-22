# W16 ENGINE

**W16 ENGINE** is a custom 3D game engine developed in C++ from scratch. This project was created for the **Game Engines** course at [CITM - UPC](https://www.citm.upc.edu/).

The engine focuses on modularity, performance optimization, and providing a user-friendly interface for scene composition and resource management.

**Powered by:** Torrats Games

---

## Development Team

![foto torrats](https://github.com/user-attachments/assets/152b3955-b6a8-4cd1-baef-afb7fbf1cd3f)

| Member | Role & Work Performed |
| :--- | :--- |
| **Marc Blánquez** | Developed the foundational bone logic, mesh-bone hierarchies, and infinite bone support. He implemented high-performance **GPU Skinning**, animation optimizations, and low-level shader/stencil management. |
| **Martí Mira** | Architected the Resource Management system (Meta-files) and the Resource Explorer. He developed asset/texture importing pipelines, asset movement logic, and optimized spatial partitioning. |
| **Arnau Balasch** |Designed the engine's interface (ImGui), including the GameObject hierarchy and Project Window search. He developed the scene serialization system (supporting animation save/load) and core camera systems. |

**GitHub Links:** [Marc Blánquez](https://github.com/MarcBlanquezPadilla) | [Martí Mira](https://github.com/algars15) | [Arnau Balasch](https://github.com/Balar05)

---

## Core Systems

The engine is built on a modular architecture inspired by industry standards:

* **Component-Based Architecture:** Every entity is a `GameObject` that can be extended with components such as `Transform`, `Camera`, and the `AnimationController`.
* **Resource Management (Meta System):** A smart system that tracks assets using `.meta` files, handling automatic imports and file hashing to detect external changes.
* **Color-Coding Mouse Picking:** Object selection is handled by encoding each GameObject's Unique ID (UID) into a specific RGB color. The scene is rendered to a hidden texture; upon clicking, the engine reads the pixel color to retrieve the selected object's UID.
* **Scene Serialization:** Full support for saving and loading scenes in a custom format, preserving the hierarchy and the state of all components, including animation data.
* **Advanced Project Window:** A resource explorer with integrated search functionality. It allows for deep inspection of complex assets; double-clicking a model enables users to explore internal sub-resources like meshes, animations, and textures.
* **Skinned Mesh Renderer (SMR):** A unified component that replaces separate Mesh and Texture components for skeletal models. Similar to Unity's SMR, it binds the mesh to a skeletal hierarchy, ensuring bone transforms drive mesh deformation.
* **Updated Render Module:** The core rendering pipeline has been refactored to support skeletal mesh data and specialized animation shaders.

---

## High-Level System: Skeletal Animation

For this release, we have implemented a high-level **Skeletal Animation System** that allows for real-time mesh deformation using bone hierarchies.

### Technical Details:
* **Hybrid Pipeline:** The skeletal hierarchy and bone transforms are calculated on the CPU, while the vertex skinning (mesh deformation) is processed on the GPU via shaders for maximum efficiency.
* **Vertex Skinning:** Real-time calculation of vertex positions based on bone weights and transforms stored in the mesh data.
* **Animation Blending & Transitions:** Smooth switching between states is achieved through **Snapshot Transitions**. The engine captures a snapshot of the current skeletal pose and interpolates it with the target animation over time.
* **Interactive Demo:** We have integrated a character into the city scene with the following logic:
    * **Idle State:** The character automatically plays an idle loop upon loading.
    * **Walk Cycle (Key '1'):** Holding the **'1'** key triggers a walking animation loop. The character seamlessly transitions back to idle when released.
    * **Attack Action (Key '2'):** Pressing the **'2'** key triggers an attack animation, which can be initiated from both Idle and Walk states.

### Visual Demonstration

| Animations | Animator Component | Mesh Renderer |
| :---: | :---: | :---: |
| ![Animations GIF](https://github.com/user-attachments/assets/bed553ab-e79b-4108-861a-f597722e7560) | ![Animator Component GIF](https://github.com/user-attachments/assets/2edc8fbc-d913-4169-8d51-6f2ac1cc62d7) | ![Mesh Renderer GIF](https://github.com/user-attachments/assets/1ccf23ab-36c0-4a48-a60c-bf9f5a81bab1) |

---

## Scene Creation

Watch the following 1-minute video showing the process of creating a scene and configuring the animation system within the W16 Engine:

[**Scene Creation Video**](https://youtu.be/70icB58xoKs)
*Click to watch the video on YouTube.*

---

## Repository & Downloads

* **Repository:** [W16 Engine on GitHub](https://github.com/MarcBlanquezPadilla/W16Engine)
* **Project Branch:** [main](https://github.com/MarcBlanquezPadilla/W16Engine/tree/main)
* **Latest Release:** [Download W16 Engine v1.0](https://github.com/MarcBlanquezPadilla/W16Engine/releases)

---

## 📜 License

This software is distributed under the **MIT License**. See the `LICENSE` file for more details.
