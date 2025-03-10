# # Donner la version minimum de CMake à utiliser,
# # Pour une compatibilité CUDA la version doit être supérieure ou égale
# # à 3.9.
# cmake_minimum_required(VERSION "3,9")

# # Ici, nul besoin de spécifier la plateforme source, la compilation 
# # ciblera automatiquement les plateformes 64 bits

# # Ensuite, il faut déclarer le projet en renseignant les langages 
# # utilisés (donc CUDA et C++ dans ce cas)
project(cuda CUDA CXX)

# On déclare ensuite que l’on veut produire une bibliothèque statique
# add_library(cudaLibProject STATIC cudalib.cu cudalib.h)
add_library(cuda STATIC cudaCompute.cu cudaCompute.h)