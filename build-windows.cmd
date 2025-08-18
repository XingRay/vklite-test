cmake -S . -B build -G "Visual Studio 17 2022" -DBUILD_SHARED_LIBS=OFF
cmake --build ./build --config Release
cmake --build ./build --config Debug
