# **Alpha Engine**

## A High Perfomance 3D Game Engine made with C++ and OpenGL.

<img width="1122" height="495" alt="AlphaEngine" src="https://github.com/user-attachments/assets/053b071e-f34b-4d44-b60c-d7a137ed8844" />
<br>

## **✨ Features**

### **Right now it has:**

* Window - Application Abstraction
* Abstraction App - UI Layer.
* OpenGL abstraction Layer
* Signature-based Entity-Component-System (ECS) + Pooling
* Events (Window Events, Input Events, Gameplay Events)
* Event Bus + Event Dispatcher
* Jolt Physics + Contact Listeners
* Input
* Asset Manager (loading Models, Textures, Shaders and having handles and jobs)
* Custom Logger
* Systems ( Render System, Physics System, PlayerControllerSystem (WASD), CameraSystem)
* Components ( Transform, PlayerControllerComponent, RigidBody )
* Service Locator Pattern
* Frustum Culling

<br>

## **🛠 To Add (Roadmap)**

* Particle System
* Lua Scripting Lang
* Unreal GAS-alike
* IMGUI - Scene ViewPort

<br>

## **📋 Prerequisites**

* C++ Compiler: Needs to support C++17 or higher
* CMake: Version 4.2 or higher
* Graphics Drivers: Ensure GPU drivers are up to date for OpenGL support.

<br>

## **📦 Dependencies (Automated)**

* GLFW (Windowing & Input)
* GLM (OpenGL Mathematics)
* Jolt Physics (Physics Engine)
* Assimp (3D Model Loading)
* stb_image (Image Loading)

<br>

## **🔨 Build**

#### **Steps:**

1. Clone Repo
2. Create new folder "build" inside the AlphaEngine Folder and navigate to your new created build folder
3. Open CMD inside your newly created build folder
4. Write: | **cmake -S .. -B .** |

This will create the **Alpha.slnx** which now you can open and Run it through your IDE (Debug/Release Mode)

## **Enjoy!**

