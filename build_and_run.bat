echo "building..."
cd %gd%
msbuild
echo "running..."
.\x64\Debug\gl0.exe
