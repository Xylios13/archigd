# ArchiGD
ArchiCAD Generative Design Tool

In archicad you can find every file for the Racket side of this project. It has a testing examples folder filled with examples that use our solution locally (main.rkt) rather than using rosetta/archicad. Word of warning, some examples might need little tweaks to work with newer versions of operations, such as different parameter names or different organization of input (Property Lists instead of list of lists in objects, etc...).

In x64 you will find the plug-in ready to use.

In Visual Studio Code you can find every file needed to be used in a solution of an ArchiCAD plug-in.

Before doing anything:
Make sure that your build is set to Release x64, normally these options are bellow TEST ANALYZE WINDOW HELP. This is required for the use of the plug-in in any computer and not just the ones you own.


I use Geometry_Test example to build my plug-in. I recommend you use the same solution, and do the following adjustments:

Directory/Files that can be replaced by the files of this repository:

In Src you can delete Geometry_Test.cpp and add all the files of our Src.
RINT replace
RFIX.win replace

Additional Include Directories:

Right-click your project on the left side of visual studio by default, and go to properties.
Go to C/C++, and in the Additional Include Directories make sure you have the correct path for all the necessary files in Support
In my solution I had the following:
..\..\Support\Inc
..\..\Support\Modules\GSRoot
..\..\Support\Modules\GSRoot\STL\imp
..\..\Support\Modules\GSUtils
..\..\Support\Modules\Geometry
..\..\Support\Modules\GDL
..\..\Support\Modules\DGLib
..\..\Support\Modules\InputOutput
..\..\Support\Modules\UCLib
..\..\Support\Modules\VectorImage
..\..\Support\Modules\VBUtils
..\..\Support\Modules\Pattern

Still in C/C++ General I have my Warning level set to Level2 and not Level4

C/C++ -> Preprocessor
Preprocessor Definitions I have 

WIN32
NDEBUG
_WINDOWS
_USRDLL
_STLP_DONT_FORCE_MSVC_LIB_NAME

Linker -> InputOutput
I have:

msvcrt.lib
msvcprt.lib
ws2_32.lib
libprotobuf.lib
libprotobuf-lite.lib
libprotoc.lib
OLDNAMES.lib
Ws2_32.lib



Check MySolutionFiles.png to see what files I have on my project, including .lib. I manually added VBUtilsImp.LIB and VectorImageImp.LIB found in API Development Kit 18.3006/Support/Modules

You need to have you personal information (given by GRAPHISOFT) in Visual Studio Code > RFIX > RosettaArchiCADFix.grc


Finally, feel free to use compile.bat and copyAddon.bat to ease your workflow. The first one compiles the messages.proto and moves the resulting files to their correct directory (archicad for racket, and src for c++), you can edit the .bat to change those directories. The second one copies the generated addon from a build done in the visual studio to my ArchiCAD folder (you need to change this folder so it matches your ArchiCAD installation).







