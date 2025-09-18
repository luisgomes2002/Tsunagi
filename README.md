# Build e Execução com CMake

This project uses **CMake** for build management.
Below are instructions for compiling and running on **Windows.**

---

## 🔧 Prerequisites

- **CMake** (>= 3.10)
- Compiler:
  - Windows → `MinGW` or `MSVC`
- **Git** (optional, if you want to clone the project directly)

---

## 🪟 Windows

```bash
# Clone the project
git clone https://github.com/luisgomes2002/Tsunagi.git
cd projeto

# Create build folder
mkdir build
cd build

# Generate build files
cmake .. -G "MinGW Makefiles"

# Compile
cmake --build .

# Run the program
./tsunagi.exe
```
