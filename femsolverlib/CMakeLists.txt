
file(GLOB source_files_lib
"include/*.h"
"src/*.cpp"
)


add_library(femsolverlib SHARED ${source_files_lib})







