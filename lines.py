# import OS module
import os
# Get the list of all files and directories

filesRead = dict()

paths = [
"./glimac/glimac/",
"./glimac/src/",
"src/"
]

exclude = [
"BBox.hpp",
"common.hpp",
"Cone.hpp",
"FilePath.hpp",
"Geometry.hpp",
"getTime.hpp",
"glm.hpp",
"Image.hpp",
"Program.hpp",
"Shader.hpp",
"Sphere.hpp",
"Sphere.hpp",

"Cone.cpp",
"desktop.ini",
"Geometry.cpp",
"Image.cpp",
"Program.cpp",
"Shader.cpp",
"Shader.cpp",
"stb_image.h",
"tiny_obj_loader.cpp",
"tiny_obj_loader.h",
"CMakeLists.txt",
]

def getLinesOfFolder(path: str):
    count = 0
    files = os.listdir(path)
    for file in files:
        if file in exclude:
            continue
        if os.path.isdir(path+file):
            count += getLinesOfFolder(str(path+file+"/"))
            continue
        with open(path+file, 'r') as f:
            nLines = len(f.readlines())
            count += nLines
            filesRead[path+file] = nLines
    return count

# count = 0
# for path in paths:
#     dir_list = os.listdir(path)
#     print(dir_list)

#     for f in dir_list:
#         with open(path+f, 'r') as f:
#             count += len(f.readlines())

count = 0
for path in paths:
    count += getLinesOfFolder(path)

for name in sorted(filesRead):
    print(f"{filesRead[name]}\t{name}")

print(count)
# fp= open(r'File_Path', 'r')